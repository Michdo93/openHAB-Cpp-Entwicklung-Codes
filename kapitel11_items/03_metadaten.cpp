// Kapitel 11: Metadaten – Zusatzinformationen, die openHABs Verhalten ändern
#include <openhab/openhab.hpp>

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
    const std::string name = "Klingel_Status";
    items.addOrUpdateItem(name, {{"type", "String"},
                                 {"name", name},
                                 {"label", "Klingel"},
                                 {"groupNames", {"gHaus"}}});

    // 1. stateDescription: Anzeigeformat und erlaubte Werte für Oberflächen
    json beschreibung = {
        {"value", " "},
        {"config",
         {{"pattern", "%s"},
          {"readOnly", false},
          {"options", "RUHE=Ruhe,KLINGELT=Es klingelt!,GEOEFFNET=Tür geöffnet"}}}};
    std::cout << "stateDescription: "
              << items.addMetadata(name, "stateDescription", beschreibung) << "\n";
    // 2. expire: Zustand fällt nach zehn Sekunden automatisch zurück
    std::cout << "expire:           "
              << items.addMetadata(name, "expire", {{"value", "10s,state=RUHE"}}) << "\n";
    // 3. eigener Namensraum: beliebige Daten für die eigene Anwendung
    json eigene = {{"value", "klingel"}, {"config", {{"raum", "Flur"}, {"prioritaet", 2}}}};
    std::cout << "eigener:          " << items.addMetadata(name, "restbuch", eigene)
              << "\n";
    std::cout << "Namensräume:      " << items.getMetadataNamespaces(name) << "\n";

    json item = items.getItem(name, "stateDescription,expire,restbuch");
    for (const auto& [ns, daten] : item["metadata"].items()) {  // je Namensraum
        std::cout << "  " << std::left << std::setw(16) << ns << " value=\""
                  << daten["value"].get<std::string>() << "\"\n";
        json config = daten.value("config", json::object());  // eigene Variable
        for (const auto& [schluessel, wert] : config.items())
            std::cout << "  " << std::setw(16) << "" << " " << schluessel << "="
                      << wert.dump() << "\n";
    }

    // Die Anzeige nutzt die Optionen aus stateDescription
    items.postUpdate(name, "KLINGELT");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    item = items.getItem(name);
    std::string state = item["state"], anzeige = "?";
    for (const json& o : item["stateDescription"]["options"])
        if (o["value"] == state) anzeige = o["label"];
    std::cout << "Zustand: " << state << " | Anzeige: " << anzeige << "\n";

    std::this_thread::sleep_for(std::chrono::seconds(11));  // expire setzt zurück
    std::cout << "nach 11 s: " << items.getItemState(name).get<std::string>() << "\n";
    std::cout << "entfernen:        " << items.removeMetadata(name, "restbuch") << "\n";
    items.deleteItem(name);
    return 0;
}
