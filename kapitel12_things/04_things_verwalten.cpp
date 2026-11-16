// Kapitel 12: Ein Thing anlegen, umkonfigurieren, abschalten und löschen
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

using openhab::json;
using namespace std::chrono_literals;

const std::string UID = "astro:moon:buch";

void status(openhab::Things& things, const std::string& was) {
    json s = things.getThingStatus(UID);
    std::string detail = s["statusDetail"];
    std::cout << std::left << std::setw(9) << was << " " << s["status"].get<std::string>()
              << (detail != "NONE" ? " / " + detail : "") << "\n";
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Things things(client);

    json neu = things.createThing(
        {{"UID", UID},
         {"thingTypeUID", "astro:moon"},
         {"label", "Mond (per REST)"},
         {"configuration", {{"geolocation", "48.0950,8.6630"}, {"interval", 300}}}});
    std::cout << "angelegt: " << neu["UID"].get<std::string>() << " mit "
              << neu["channels"].size() << " Channels\n";
    std::this_thread::sleep_for(2s);
    status(things, "Status:");

    // Konfiguration ändern – openHAB initialisiert das Thing neu
    things.updateThingConfiguration(UID,
                                    {{"geolocation", "52.5200,13.4050"}, {"interval", 60}});
    json thing = things.getThing(UID);
    std::cout << "Ort:      " << openhab::str(thing, "configuration.geolocation")
              << " (Berlin)\n";

    // Label ändern: updateThing ersetzt die ganze Definition
    thing["label"] = "Mond über Berlin";
    std::cout << "Label:    " << openhab::str(things.updateThing(UID, thing), "label")
              << "\n";

    // Abschalten und wieder einschalten
    things.disableThing(UID);
    std::this_thread::sleep_for(1s);
    status(things, "aus:");
    things.enableThing(UID);
    std::this_thread::sleep_for(2s);
    status(things, "an:");

    std::cout << "löschen:  " << openhab::str(things.deleteThing(UID), "statusInfo.status")
              << "\nThings:  ";
    for (const json& t : things.getThings(true))
        std::cout << " " << t["UID"].get<std::string>();
    std::cout << "\n";
    return 0;
}
