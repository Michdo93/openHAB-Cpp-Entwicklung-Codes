// Kapitel 10: Commands senden
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);
    std::vector<std::pair<std::string, std::string>> befehle = {
        {"Wohnzimmer_Licht", "ON"},              // Switch
        {"Wohnzimmer_Dimmer", "60"},             // Dimmer: Prozent
        {"Wohnzimmer_Farblicht", "240,100,80"},  // Color: Farbton,Sättigung,Helligkeit
        {"Kueche_Rollladen", "DOWN"},            // Rollershutter
        {"Haus_Status", "Alle zu Hause"},        // String
    };
    for (const auto& [item, command] : befehle) {
        openhab::json antwort = items.sendCommand(item, command, "buch:kapitel10");
        std::cout << std::left << std::setw(22) << item << " <- " << std::setw(14)
                  << command << " " << antwort.dump() << "\n";
    }
    // openHAB arbeitet asynchron: kurz warten, dann die neuen Zustände lesen
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    for (const auto& [item, command] : befehle)
        std::cout << std::setw(22) << item << " = "
                  << items.getItemState(item).get<std::string>() << "\n";
    return 0;
}
