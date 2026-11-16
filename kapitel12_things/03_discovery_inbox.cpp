// Kapitel 12: Geräte suchen lassen und aus der Inbox übernehmen
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

using openhab::json;
using namespace std::chrono_literals;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Services services(client);

    // Das Astro-Binding findet "Geräte" für den eingestellten Standort.
    // Der Standort ist eine Systemeinstellung – auch sie lässt sich per REST setzen.
    json config = services.getServiceConfig("org.openhab.i18n");
    config["location"] = "48.0950,8.6630";  // Breite, Länge
    services.updateServiceConfig("org.openhab.i18n", config);
    std::cout << "Standort: "
              << openhab::str(services.getServiceConfig("org.openhab.i18n"), "location")
              << "\n";

    openhab::Discovery discovery(client);
    std::cout << "Bindings mit Gerätesuche: " << discovery.getDiscoveryBindings() << "\n";
    std::cout << "Suche läuft, Dauer in s: " << discovery.startBindingScan("astro") << "\n";
    std::this_thread::sleep_for(3s);
    openhab::Inbox inbox(client);
    for (const json& e : inbox.getDiscoveredThings())
        std::cout << "  " << std::left << std::setw(8) << e["flag"].get<std::string>()
                  << " " << std::setw(18) << e["thingUID"].get<std::string>() << " "
                  << std::setw(20) << e["label"].get<std::string>() << " "
                  << openhab::str(e, "properties.geolocation") << "\n";

    // Den Mond übernehmen: aus dem Inbox-Eintrag wird ein Thing
    std::cout << "\nübernehmen: "
              << inbox.approveDiscoveryResult("astro:moon:local", "Mond über Aldingen")
              << "\n";
    std::this_thread::sleep_for(2s);
    openhab::Things things(client);
    json mond = things.getThing("astro:moon:local");
    std::cout << mond["label"].get<std::string>() << " "
              << openhab::str(mond, "statusInfo.status") << " " << mond["channels"].size()
              << " Channels\n";

    // Wieder entfernen – der Eintrag erscheint bei der nächsten Suche erneut
    std::cout << "löschen: "
              << openhab::str(things.deleteThing("astro:moon:local"), "statusInfo.status")
              << "\n";
    return 0;
}
