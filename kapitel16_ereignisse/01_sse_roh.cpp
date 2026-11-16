// Kapitel 16: Den Ereignisstrom direkt lesen – Zeile für Zeile
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");

    // Alle Zustandsänderungen aller Items (Topic openhab/items/*/statechanged)
    openhab::EventStream strom = openhab::ItemEvents(client).ItemStateChangedEvent();
    if (strom.isError()) {
        std::cerr << strom.error() << "\n";
        return 1;
    }
    std::thread dimmer([&client] {  // nach einer Sekunde etwas ändern
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string wert = std::to_string(20 + std::time(nullptr) % 60);
        openhab::Items(client).sendCommand("Wohnzimmer_Dimmer", wert);
    });
    std::cout << "Warte auf Ereignisse …\n";

    auto ende = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    while (std::chrono::steady_clock::now() < ende) {
        std::optional<std::string> zeile = strom.nextLine(1000);  // höchstens 1 s warten
        if (!zeile) {
            if (!strom.isOpen()) break;  // Strom zu Ende
            continue;                    // nur Zeit abgelaufen
        }
        std::cout << "roh: " << zeile->substr(0, 70) << "\n";
        // 1. JSON: das Ereignis; die Bibliothek entpackt auch payload (2. JSON)
        std::optional<openhab::json> e = openhab::EventStream::parseData(*zeile);
        if (e && e->contains("topic")) {  // event:, id:, "" und ALIVE übergehen
            std::string topic = (*e)["topic"];
            std::string name = topic.substr(14, topic.find('/', 14) - 14);
            std::cout << "  -> " << name << ": " << openhab::str(*e, "payload.oldValue")
                      << " -> " << openhab::str(*e, "payload.value") << "\n";
        }
    }
    strom.close();
    dimmer.join();
    std::cout << "beendet\n";
    return 0;
}
