// Kapitel 11: Gruppen zur Laufzeit ändern und den Zustandsverlauf lesen
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using openhab::json;

// Alle Ketten von Gruppen über einem Item sammeln, z. B. gLichter -> gHaus (rekursiv)
void ketten(const json& item, const std::string& bisher, std::vector<std::string>& ziel) {
    const json eltern = item.value("parents", json::array());
    if (eltern.empty()) {
        if (!bisher.empty()) ziel.push_back(bisher);
        return;
    }
    for (const json& p : eltern)
        ketten(p, bisher + (bisher.empty() ? "" : " -> ") + p["name"].get<std::string>(),
               ziel);
}

// Millisekunden seit 1970 als Uhrzeit
std::string uhrzeit(double millis) {
    std::time_t t = static_cast<std::time_t>(millis / 1000);
    std::ostringstream s;
    s << std::put_time(std::localtime(&t), "%H:%M:%S");
    return s.str();
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);

    // Ein per REST angelegtes Licht in die Gruppe aller Lichter aufnehmen
    items.addOrUpdateItem("Flur_Licht", {{"type", "Switch"},
                                         {"name", "Flur_Licht"},
                                         {"label", "Flurlicht"},
                                         {"tags", {"Switch", "Light"}}});
    std::cout << "aufnehmen: "
              << openhab::str(items.addGroupMember("gLichter", "Flur_Licht"), "message")
              << "\nMitglieder:";
    json gruppe = items.getItem("gLichter");  // erst speichern, dann darüber laufen
    for (const json& m : gruppe["members"])
        std::cout << " " << m["name"].get<std::string>();
    std::cout << "\n";

    // Ein Command an die Gruppe erreicht alle Mitglieder
    items.sendCommand("gLichter", "OFF");
    items.sendCommand("Flur_Licht", "ON");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << "gLichter: " << items.getItemState("gLichter").get<std::string>()
              << " (mindestens eines ist an)\n";

    // openHAB 5 merkt sich den vorherigen Zustand und die Zeitpunkte
    json licht = items.getItem("Flur_Licht");
    std::cout << "Flur_Licht: " << openhab::str(licht, "state") << " (vorher "
              << openhab::str(licht, "lastState") << "), aktualisiert "
              << uhrzeit(openhab::num(licht, "lastStateUpdate")) << ", geändert "
              << uhrzeit(openhab::num(licht, "lastStateChange")) << "\n";

    // In welchen Gruppen steckt das Item – bis ganz nach oben?
    openhab::Items::GetItemArgs mitEltern;
    mitEltern.parents = true;
    std::vector<std::string> alle;
    ketten(items.getItem("Flur_Licht", mitEltern), "", alle);
    std::cout << "Eltern:";
    for (size_t i = 0; i < alle.size(); i++) std::cout << (i ? " | " : " ") << alle[i];
    std::cout << "\n";

    std::cout << "entfernen: "
              << openhab::str(items.removeGroupMember("gLichter", "Flur_Licht"), "message")
              << "\n";
    items.deleteItem("Flur_Licht");
    return 0;
}
