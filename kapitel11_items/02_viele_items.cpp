// Kapitel 11: Einen ganzen Raum mit einem einzigen Aufruf anlegen
#include <openhab/openhab.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);

    // Rohtext-Literal R"(…)": JSON ohne Escape-Zeichen
    json gartenhaus = json::parse(R"([
      {"type": "Group", "name": "gGartenhaus", "label": "Gartenhaus",
       "groupNames": ["gHaus"], "tags": ["Garden"]},
      {"type": "Switch", "name": "Gartenhaus_Licht", "label": "Licht",
       "groupNames": ["gGartenhaus", "gGartenhaus_Lichter"],
       "tags": ["Switch", "Light"], "category": "light"},
      {"type": "Number:Temperature", "name": "Gartenhaus_Temperatur",
       "label": "Temperatur", "groupNames": ["gGartenhaus"],
       "tags": ["Measurement", "Temperature"], "category": "temperature"},
      {"type": "Contact", "name": "Gartenhaus_Tuer", "label": "Tür",
       "groupNames": ["gGartenhaus"], "tags": ["OpenState", "Door"]},
      {"type": "Group", "name": "gGartenhaus_Lichter", "label": "Lichter im Gartenhaus",
       "groupNames": ["gGartenhaus"], "groupType": "Switch",
       "function": {"name": "OR", "params": ["ON", "OFF"]}}])");

    for (const json& r : items.addOrUpdateItems(gartenhaus))  // ein PUT für alle
        std::cout << "anlegen: " << std::left << std::setw(24)
                  << r["name"].get<std::string>() << " " << r["status"].get<std::string>()
                  << "\n";

    items.postUpdate("Gartenhaus_Temperatur", "12.5 °C");
    items.postUpdate("Gartenhaus_Tuer", "CLOSED");
    items.sendCommand("Gartenhaus_Licht", "ON");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    openhab::Items::GetItemArgs rekursiv;
    rekursiv.recursive = true;
    json mitglieder = items.getItem("gGartenhaus", rekursiv)["members"];
    std::sort(mitglieder.begin(), mitglieder.end(),
              [](const json& a, const json& b) { return a["name"] < b["name"]; });
    for (const json& m : mitglieder)
        std::cout << "  " << std::setw(24) << m["name"].get<std::string>() << std::setw(20)
                  << m["type"].get<std::string>() << " " << m["state"].get<std::string>()
                  << "\n";

    // Die Gruppenfunktion OR(ON,OFF) fasst die Mitglieder zusammen
    std::cout << "Lichter im Gartenhaus: "
              << items.getItemState("gGartenhaus_Lichter").get<std::string>() << "\n";

    // Aufräumen: von hinten nach vorn – erst die Mitglieder, dann die Gruppen
    for (auto it = gartenhaus.rbegin(); it != gartenhaus.rend(); ++it)
        items.deleteItem((*it)["name"].get<std::string>());
    std::cout << "aufgeräumt: "
              << (openhab::isError(items.getItem("gGartenhaus")) ? "ja" : "nein") << "\n";
    return 0;
}
