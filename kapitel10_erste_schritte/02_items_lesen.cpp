// Kapitel 10: Items lesen – alle, eines, nur den Zustand, gefiltert
#include <openhab/openhab.hpp>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);

    // Alle Items, aber nur drei Felder – das spart Daten
    openhab::Items::GetItemsArgs nurDrei;
    nurDrei.fields = "name,type,state";
    json alle = items.getItems(nurDrei);
    if (openhab::isError(alle)) {
        std::cerr << "Fehler: " << openhab::errorMessage(alle) << "\n";
        return 1;
    }
    std::sort(alle.begin(), alle.end(),
              [](const json& a, const json& b) { return a["name"] < b["name"]; });
    std::cout << alle.size() << " Items gefunden:\n";
    for (const json& i : alle)
        std::cout << "  " << std::left << std::setw(28) << i["name"].get<std::string>()
                  << std::setw(22) << i["type"].get<std::string>()
                  << i["state"].get<std::string>() << "\n";

    // Ein Item mit allen Details
    json licht = items.getItem("Wohnzimmer_Licht");
    std::cout << "\nLabel: " << licht["label"].get<std::string>() << " | Gruppen:";
    for (const json& g : licht["groupNames"]) std::cout << " " << g.get<std::string>();
    std::cout << "\nTags:";
    for (const json& t : licht["tags"]) std::cout << " " << t.get<std::string>();

    // Nur der Zustand: ein JSON-String
    json temp = items.getItemState("Wohnzimmer_Temperatur");
    std::cout << "\nTemperatur: " << temp.get<std::string>() << "\n";

    // Filter: nach Typ (erster Parameter) und nach Tag
    json dimmer = items.getItems("Dimmer", std::nullopt, ".*", false, "name");
    std::cout << "Dimmer: " << openhab::str(dimmer, "0.name") << "\n";
    openhab::Items::GetItemsArgs mitTag;
    mitTag.tags = "Light";
    mitTag.fields = "name";
    std::cout << "Mit Tag 'Light':";
    for (const json& i : items.getItems(mitTag))
        std::cout << " " << i["name"].get<std::string>();
    std::cout << "\n";
    return 0;
}
