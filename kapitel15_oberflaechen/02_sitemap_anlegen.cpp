// Kapitel 15: Eine Sitemap als UI-Komponente erzeugen
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iostream>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::UI ui(client);
    const std::string uid = "buch_tablet";

    json tablet = json::parse(R"({
      "uid": "buch_tablet", "component": "Sitemap", "config": {"label": "Tablet im Flur"},
      "slots": {"widgets": [
        {"component": "Frame", "config": {"label": "Licht"}, "slots": {"widgets": [
          {"component": "Switch", "config": {"item": "Wohnzimmer_Licht", "label": "Decke"}},
          {"component": "Slider",
           "config": {"item": "Wohnzimmer_Dimmer", "label": "Leselampe"}},
          {"component": "Switch", "config": {"item": "gLichter", "label": "Alle Lichter",
                                             "mappings": ["OFF=Alle aus"]}}]}},
        {"component": "Frame", "config": {"label": "Haus"}, "slots": {"widgets": [
          {"component": "Text", "config": {"item": "Flur_Haustuer", "label": "Haustür"}},
          {"component": "Selection", "config": {"item": "Haus_Status", "label": "Modus",
           "mappings": ["Alle zu Hause=Zu Hause", "Kino=Kino",
                        "Gute Nacht=Nacht"]}}]}}]}})");
    ui.deleteUIComponent("system:sitemap", uid);  // falls vom letzten Lauf vorhanden
    std::cout << "anlegen: "
              << openhab::str(ui.addUIComponent("system:sitemap", tablet), "uid") << "\n";

    openhab::Sitemaps sitemaps(client);
    std::cout << "Sitemaps jetzt:";
    for (const json& s : sitemaps.getSitemaps())
        std::cout << " " << s["name"].get<std::string>();
    std::cout << "\n";

    json sitemap = sitemaps.getSitemap(uid);
    for (const json& frame : sitemap["homepage"]["widgets"]) {
        std::cout << "  " << frame["label"].get<std::string>() << ":";
        for (const json& w : frame["widgets"])
            std::cout << " " << w["label"].get<std::string>() << " ("
                      << w["type"].get<std::string>() << ")";
        std::cout << "\n";
    }

    std::cout << "UI-Komponenten:";
    for (const json& k : ui.getUIComponents("system:sitemap"))
        std::cout << " " << k["uid"].get<std::string>();
    std::cout << "\n";
    // Die Sitemap bleibt bestehen – 03_sitemap_ereignisse nutzt sie.
    return 0;
}
