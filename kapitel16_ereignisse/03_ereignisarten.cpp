// Kapitel 16: Ein Strom für vieles – Items, Things und Links gleichzeitig beobachten
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

const std::string SONNE = "astro:sun:zuhause";
const std::string KANAL = SONNE + ":position#azimuth";

// Ausgabe je Ereignisart: die payload ist je nach Typ anders aufgebaut
std::string info(const std::string& typ, const openhab::json& p) {
    if (typ == "ThingStatusInfoChangedEvent")  // payload: [neu, alt]
        return openhab::str(p, "1.status") + " -> " + openhab::str(p, "0.status") + " (" +
               openhab::str(p, "0.statusDetail") + ")";
    if (typ.rfind("ItemChannelLink", 0) == 0) {
        std::string kanal = openhab::str(p, "channelUID");
        return openhab::str(p, "itemName") + " <-> " + kanal.substr(kanal.rfind(':') + 1);
    }
    if (p.contains("value")) return openhab::str(p, "value");
    return openhab::str(p, "name");
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    std::string themen =
        "openhab/items/Sonne_Azimut/added,"         // Item angelegt
        "openhab/items/Sonne_Azimut/removed,"       // Item gelöscht
        "openhab/items/Sonne_Azimut/statechanged,"  // Zustand
        "openhab/things/*/statuschanged,"           // alle Things
        "openhab/links/*/*";                        // Links
    openhab::EventStream strom = openhab::Events(client).getEvents(themen);

    // Erzeugt nacheinander typische Ereignisse und unterbricht dann den Strom
    std::thread aktionen([&] {
        openhab::Items items(client);
        openhab::Links links(client);
        openhab::Things things(client);
        std::this_thread::sleep_for(1s);
        items.addOrUpdateItem("Sonne_Azimut", {{"type", "Number:Angle"},
                                               {"name", "Sonne_Azimut"},
                                               {"label", "Sonnenrichtung"}});
        links.linkItemToChannel("Sonne_Azimut", KANAL, openhab::json::object());
        std::this_thread::sleep_for(1s);
        things.disableThing(SONNE);
        std::this_thread::sleep_for(1s);
        things.enableThing(SONNE);
        std::this_thread::sleep_for(2s);
        links.unlinkItemFromChannel("Sonne_Azimut", KANAL);
        items.deleteItem("Sonne_Azimut");
        std::this_thread::sleep_for(1s);
        strom.interrupt();  // weckt den Leser, die Schleife endet
    });
    for (const openhab::json& e : strom) {  // range-for: ohne Zeitgrenze
        std::string typ = e["type"];
        std::cout << std::left << std::setw(35) << typ << info(typ, e["payload"]) << "\n";
    }
    aktionen.join();
    return 0;
}
