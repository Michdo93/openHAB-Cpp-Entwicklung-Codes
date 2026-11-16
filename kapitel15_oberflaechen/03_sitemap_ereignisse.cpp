// Kapitel 15: Änderungen auf einer Sitemap-Seite live verfolgen
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using openhab::json;
const std::string SITEMAP = "buch_tablet";  // aus 02_sitemap_anlegen.cpp

std::string links(const std::string& s, size_t breite) {  // wie in 01_sitemap_lesen
    size_t zeichen = 0;
    for (unsigned char z : s) zeichen += (z & 0xC0) != 0x80;
    return zeichen < breite ? s + std::string(breite - zeichen, ' ') : s;
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Sitemaps sitemaps(client);
    openhab::Items items(client);

    // 1. Abonnement anlegen und an eine Seite binden: die Seite mit der ID abrufen
    std::string id = sitemaps.subscribeToSitemapEvents()["subscriptionID"];
    json seite = sitemaps.getSitemapPage(SITEMAP, SITEMAP, id);
    std::cout << "Abonnement " << id.substr(0, 8) << "… für Seite »"
              << seite["title"].get<std::string>() << "«\n";

    // openHAB antwortet auf den Ereignisstrom erst mit dem ersten Ereignis –
    // deshalb ändert ein zweiter Thread etwas, während der Hauptthread den Strom öffnet
    std::thread aendern([&items] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        bool war35 = items.getItemState("Wohnzimmer_Dimmer") == "35";
        items.sendCommand("Wohnzimmer_Dimmer", war35 ? "65" : "35");
        items.postUpdate("Flur_Haustuer", "OPEN");
    });
    // 2. Den Strom lesen: Sitemap-Ereignisse haben kein "topic" – includeAlive = true
    openhab::EventStream strom = sitemaps.getSitemapEvents(id, SITEMAP, SITEMAP);
    int anzahl = 0;
    while (anzahl < 3) {
        auto e = strom.nextEvent(10000, true);  // höchstens 10 s warten
        if (!e) break;                          // Zeit abgelaufen oder Fehler
        if (!e->contains("widgetId")) continue;
        std::cout << "  Widget " << links((*e)["widgetId"], 5) << " "
                  << links((*e)["label"], 24) << " Zustand "
                  << openhab::str(*e, "item.state") << "\n";
        anzahl++;
    }
    strom.close();
    aendern.join();

    items.postUpdate("Flur_Haustuer", "CLOSED");
    std::cout << "Sitemap löschen: "
              << openhab::UI(client).deleteUIComponent("system:sitemap", SITEMAP) << "\n";
    return 0;
}
