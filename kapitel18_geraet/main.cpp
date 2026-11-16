// Kapitel 18: ohgeraet – der Raspberry Pi als Gerät für openHAB
//   ohgeraet [--intervall=30] [--ausgang=attrappe | --ausgang=gpiochip0:17] [--dauer=s]
// Ein einziger Thread: Das Warten auf Commands IST der Takt. nextEvent() wartet
// höchstens bis zur nächsten fälligen Messung.
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

#include "geraet.hpp"

namespace {
volatile std::sig_atomic_t aktiv = 1;
std::atomic<openhab::EventStream*> strom{nullptr};

extern "C" void beenden(int) {
    aktiv = 0;
    if (openhab::EventStream* s = strom.load()) s->interrupt();
}

void logZeile(const std::string& text) {
    char uhr[16];
    std::time_t t = std::time(nullptr);
    std::strftime(uhr, sizeof uhr, "%H:%M:%S", std::localtime(&t));
    std::cout << uhr << " " << text << std::endl;  // endl: journald sieht sie sofort
}

void messen(Geraet& g, Messwerte& messwerte) {
    Messung m = messwerte.lesen();
    g.melden(m);
    logZeile("gemeldet: " + (m.temperatur ? mitEinheit(*m.temperatur, 1, "°C") : "–") +
             ", Last " + mitEinheit(m.last.value_or(0), 0, "%") + ", Speicher " +
             mitEinheit(m.speicher.value_or(0), 0, "%") + ", Netz " +
             mitEinheit(m.netzKbit.value_or(0), 1, "kbit/s"));
}
}  // namespace

int main(int argc, char** argv) {
    std::string beschreibung = "attrappe";
    int intervall = 30, dauer = 0;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.rfind("--intervall=", 0) == 0)
            intervall = std::stoi(arg.substr(12));
        else if (arg.rfind("--ausgang=", 0) == 0)
            beschreibung = arg.substr(10);
        else if (arg.rfind("--dauer=", 0) == 0)
            dauer = std::stoi(arg.substr(8));
    }
    struct sigaction sa = {};
    sa.sa_handler = beenden;
    for (int sig : {SIGINT, SIGTERM, SIGALRM}) sigaction(sig, &sa, nullptr);  // TERM: stop
    if (dauer) alarm(dauer);

    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    std::unique_ptr<Ausgang> luefter = ausgangOeffnen(beschreibung);
    if (!luefter) return 1;
    Geraet g(client, *luefter, intervall);
    Messwerte messwerte;
    char rechner[64] = "?";
    gethostname(rechner, sizeof rechner);

    int pause = 1;
    while (aktiv) {
        // (Neu-)Anmelden und den Strom der Commands an die eigenen Items öffnen
        openhab::EventStream s;
        if (g.anmelden(rechner)) {
            s = openhab::Events(client).getEvents(
                "openhab/items/Pi_Luefter/command,openhab/items/Pi_Aktion/command");
            if (s.isError()) logZeile("kein Ereignisstrom: " + s.error());
        }
        if (s.isOpen()) {
            strom = &s;
            logZeile("angemeldet als gPi (" + std::string(rechner) + "), Ausgang " +
                     beschreibung + ", alle " + std::to_string(intervall) + " s");
            pause = 1;
            auto faellig = std::chrono::steady_clock::now();
            while (aktiv && s.isOpen()) {
                auto jetzt = std::chrono::steady_clock::now();
                if (jetzt >= faellig) {
                    messen(g, messwerte);
                    faellig = jetzt + std::chrono::seconds(intervall);
                }
                auto warten = std::chrono::duration_cast<std::chrono::milliseconds>(
                    faellig - std::chrono::steady_clock::now());
                auto e = s.nextEvent(std::max<int>(0, static_cast<int>(warten.count())));
                if (!e) continue;  // Takt erreicht oder Strom zu Ende
                std::string topic = (*e)["topic"];
                std::string item = topic.substr(14, topic.find('/', 14) - 14);
                std::string befehl = openhab::str(*e, "payload.value");
                if (auto antwort = g.command(item, befehl))
                    logZeile("Command " + item + " <- " + befehl + ": " + *antwort);
            }
            strom = nullptr;
            if (!aktiv) break;
            logZeile("Verbindung verloren");
        }
        if (!aktiv) break;
        sleep(pause);
        pause = std::min(pause * 2, 60);
    }
    g.abmelden();
    logZeile("beendet, Pi_Online = OFF");
    return 0;
}
