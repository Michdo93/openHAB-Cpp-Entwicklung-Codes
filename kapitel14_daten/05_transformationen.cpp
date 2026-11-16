// Kapitel 14: Ein Add-on installieren und eine Transformation anlegen
#include <openhab/openhab.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

using openhab::json;
using namespace std::chrono_literals;
const std::string UID = "config:map:tuer";

bool mapBereit(openhab::Transformations& t) {
    json dienste = t.getTransformationServices();
    return std::find(dienste.begin(), dienste.end(), "MAP") != dienste.end();
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Transformations transformations(client);
    openhab::Items items(client);

    // 1. Den MAP-Transformationsdienst installieren (falls noch nicht geschehen)
    openhab::Addons addons(client);
    if (!addons.getAddon("transformation-map").value("installed", false)) {
        std::cout << "installieren: " << addons.installAddon("transformation-map") << "\n";
        while (!mapBereit(transformations)) std::this_thread::sleep_for(1s);
    }
    std::cout << "Transformationsdienste: " << transformations.getTransformationServices()
              << "\n";

    // 2. Eine eigene MAP-Transformation anlegen
    json t = {{"uid", UID},
              {"label", "Türzustand"},
              {"type", "map"},
              {"configuration",
               {{"function", "OPEN=offen\nCLOSED=geschlossen\nNULL=unbekannt\n"}}}};
    std::cout << "anlegen: " << transformations.updateTransformation(UID, t) << "\n";
    for (const json& x : transformations.getTransformations())
        std::cout << "  " << std::left << std::setw(16) << x["uid"].get<std::string>()
                  << " " << std::setw(4) << x["type"].get<std::string>() << " "
                  << (x.value("editable", false) ? "änderbar" : "aus Datei") << "\n";

    // 3. Im Anzeigeformat eines Items verwenden
    items.addMetadata("Flur_Haustuer", "stateDescription",
                      {{"value", " "}, {"config", {{"pattern", "MAP(" + UID + "):%s"}}}});
    for (const char* zustand : {"OPEN", "CLOSED"}) {
        items.postUpdate("Flur_Haustuer", zustand);
        std::this_thread::sleep_for(300ms);  // postUpdate wird asynchron verarbeitet
        json tuer = items.getItem("Flur_Haustuer");
        std::cout << "Zustand " << std::setw(6) << tuer["state"].get<std::string>()
                  << " -> Anzeige " << openhab::str(tuer, "transformedState") << "\n";
    }
    // Aufräumen – das Add-on bleibt installiert
    items.removeMetadata("Flur_Haustuer", "stateDescription");
    std::cout << "löschen: " << transformations.deleteTransformation(UID) << "\n";
    return 0;
}
