// Kapitel 16: Ein robuster Ereignis-Listener – Neuverbinden, Wachhund, sauberes Beenden
//   ./starte.sh kapitel16_ereignisse/05_robuster_listener.cpp [--trennen] [--dauer=20]
#include <openhab/openhab.hpp>

#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

constexpr int WACHHUND_MS = 25000;  // openHAB schickt alle 10 s ein ALIVE

volatile std::sig_atomic_t aktiv = 1;
std::atomic<openhab::EventStream*> strom{nullptr};  // für den Signal-Handler

// Eine Zeile mit Uhrzeit davor – ein variadisches Template statt <cstdarg>
template <class... Teile>
void logZeile(const Teile&... teile) {
    char uhr[16];
    std::time_t t = std::time(nullptr);
    std::strftime(uhr, sizeof uhr, "%H:%M:%S", std::localtime(&t));
    std::cout << uhr << " ";
    (std::cout << ... << teile) << std::endl;
}

// SIGINT/SIGTERM beenden, SIGALRM (--dauer) ebenso, SIGUSR1 (--trennen) kappt nur
extern "C" void signalHandler(int sig) {
    if (sig != SIGUSR1) aktiv = 0;
    if (openhab::EventStream* s = strom.load()) s->interrupt();  // signal-sicher
}

void verarbeiten(const openhab::json& e) {
    std::string topic = e["topic"];
    std::string name = topic.substr(14, topic.find('/', 14) - 14);
    logZeile(name, " = ", openhab::str(e, "payload.value"));
}

int main(int argc, char** argv) {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    struct sigaction sa = {};
    sa.sa_handler = signalHandler;
    for (int sig : {SIGINT, SIGTERM, SIGALRM, SIGUSR1}) sigaction(sig, &sa, nullptr);
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--trennen")  // nach 3 s die Leitung kappen
            std::thread([] {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                logZeile("TEST: Verbindung gekappt");
                kill(getpid(), SIGUSR1);
            }).detach();
        else if (arg.rfind("--dauer=", 0) == 0)
            alarm(std::stoi(arg.substr(8)));
    }

    openhab::Events events(client);
    int pause = 1;
    while (aktiv) {
        openhab::EventStream s = events.getEvents("openhab/items/*/statechanged");
        if (s.isError()) {
            std::string text = s.error();
            text = text.substr(0, text.find('('));  // ohne die Adresse
            text.erase(text.find_last_not_of(' ') + 1);
            logZeile("keine Verbindung: ", text, " – neuer Versuch in ", pause, " s");
        } else {
            strom = &s;
            logZeile("verbunden");
            pause = 1;  // Erfolg: Wartezeit zurücksetzen
            // Wachhund: Kommt WACHHUND_MS lang nichts – nicht einmal ALIVE –, ist die
            // Verbindung tot, obwohl sie offen aussieht
            while (std::optional<openhab::json> e = s.nextEvent(WACHHUND_MS, true))
                if (e->contains("topic")) verarbeiten(*e);
            if (aktiv && s.isOpen())
                logZeile("Wachhund: keine Nachricht – Verbindung wird neu aufgebaut");
            else if (aktiv)
                logZeile("Verbindung beendet");
            strom = nullptr;
        }
        if (!aktiv) break;
        sleep(pause);                     // ein Signal weckt sleep() vorzeitig
        pause = std::min(pause * 2, 30);  // exponentielles Backoff bis 30 s
    }
    logZeile("Listener beendet");
    return 0;
}
