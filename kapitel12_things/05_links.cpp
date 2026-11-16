// Kapitel 12: Items mit Channels verknüpfen – mit und ohne Profil
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);
    openhab::Links links(client);
    const std::string sonne = "astro:sun:zuhause";

    // Zwei neue Items für Sonnenhöhe und Tagesphase
    items.addOrUpdateItem("Sonne_Hoehe", {{"type", "Number:Angle"},
                                          {"name", "Sonne_Hoehe"},
                                          {"label", "Sonnenhöhe"},
                                          {"groupNames", {"gHaus"}}});
    items.addOrUpdateItem("Sonne_Phase", {{"type", "String"},
                                          {"name", "Sonne_Phase"},
                                          {"label", "Tagesphase"},
                                          {"groupNames", {"gHaus"}}});

    // Welche Profile passen zwischen diesen Channel und ein Number:Angle-Item?
    std::cout << "passende Profile:";
    for (const json& p :
         openhab::ProfileTypes(client).getProfileTypes("astro:elevation", "Number:Angle"))
        std::cout << " " << p["uid"].get<std::string>();
    std::cout << "\n";

    // Link ohne Profil und Link mit Profil "offset" (+90° -> 0° bis 180°)
    std::cout << "Link 1: "
              << links.linkItemToChannel("Sonne_Phase", sonne + ":phase#name",
                                         json::object())
              << "\n";
    json profil = {{"profile", "system:offset"}, {"offset", "90 °"}};
    std::cout << "Link 2: "
              << links.linkItemToChannel("Sonne_Hoehe", sonne + ":position#elevation",
                                         profil)
              << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));  // Binding liefert den Wert
    std::cout << "Sonne_Phase = " << items.getItemState("Sonne_Phase").get<std::string>()
              << "\nSonne_Hoehe = " << items.getItemState("Sonne_Hoehe").get<std::string>()
              << " (Höhe + 90°)\n";

    json gespeichert = links.getLinks(std::nullopt, "Sonne_Hoehe");  // channelUID, item
    for (const json& l : gespeichert)
        std::cout << "gespeichert: " << l["itemName"].get<std::string>() << " -> "
                  << l["channelUID"].get<std::string>() << " " << l["configuration"].dump()
                  << "\n";

    // Aufräumen: Links lösen, Items löschen, verwaiste Links prüfen
    std::cout << "lösen: "
              << links.unlinkItemFromChannel("Sonne_Phase", sonne + ":phase#name") << "\n";
    std::cout << "alle Links des Items löschen: " << links.deleteAllLinks("Sonne_Hoehe")
              << "\n";
    items.deleteItem("Sonne_Hoehe");
    items.deleteItem("Sonne_Phase");
    std::cout << "verwaiste Links: " << links.getOrphanLinks() << "\n";
    return 0;
}
