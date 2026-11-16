// Kapitel 15: Sitemaps lesen – die Bedienseiten, die Apps und Basic UI zeigen
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

using openhab::json;

// setw zählt Bytes: Umlaute belegen in UTF-8 zwei – selbst mit Leerzeichen auffüllen
std::string links(const std::string& s, size_t breite) {
    size_t zeichen = 0;
    for (unsigned char z : s) zeichen += (z & 0xC0) != 0x80;  // Folgebytes nicht zählen
    return zeichen < breite ? s + std::string(breite - zeichen, ' ') : s;
}

// Rekursiv: jedes Widget mit Typ, Beschriftung und Zustand, Unter-Widgets eingerückt
void zeige(const json& widgets, int tiefe) {
    for (const json& w : widgets) {
        std::string state = openhab::str(w, "item.state"), label = w["label"];
        std::cout << std::string(2 * tiefe, ' ') << links(w["type"], 12) << " "
                  << (state.empty() ? label : links(label, 28) + " = " + state) << "\n";
        zeige(w.value("widgets", json::array()), tiefe + 1);
    }
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Sitemaps sitemaps(client);

    for (const json& s : sitemaps.getSitemaps())
        std::cout << "Sitemap " << s["name"].get<std::string>() << ": "
                  << s["label"].get<std::string>() << "\n";

    openhab::Sitemaps::GetSitemapArgs deutsch;
    deutsch.language = "de";
    json sitemap = sitemaps.getSitemap("restbuch", deutsch);
    std::cout << "\nStartseite »" << openhab::str(sitemap, "homepage.title") << "«:\n";
    zeige(sitemap["homepage"]["widgets"], 1);
    return 0;
}
