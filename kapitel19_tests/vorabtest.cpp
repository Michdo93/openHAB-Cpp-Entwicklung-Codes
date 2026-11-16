// Kapitel 19: Vorab-Test, bevor ein Dienst startet – oder der Roboter seine Vorführung.
// Prüft in Sekunden alles, was Dashboard (Kapitel 17) und Gerät (Kapitel 18) voraussetzen,
// und endet mit Exit-Code 0 (bereit) oder 1 (nicht starten!).
// Ideal als ExecStartPre= in einer systemd-Unit oder als Schritt einer CI-Pipeline.
#include <openhab/testsuite.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

int main() {
    const std::vector<std::pair<std::string, std::string>> ITEMS = {
        {"Wohnzimmer_Licht", "Switch"},
        {"Wohnzimmer_Dimmer", "Dimmer"},
        {"Kueche_Rollladen", "Rollershutter"},
        {"Werkstatt_Luefter", "Switch"},
        {"Werkstatt_Temperatur", "Number:Temperature"},
        {"Haus_Status", "String"},
    };
    const std::vector<std::string> THINGS = {"astro:sun:zuhause"};
    const std::vector<std::string> REGELN = {"restbuch-1"};
    const std::vector<std::string> AUF_DER_SITEMAP = {"Werkstatt_Luefter",
                                                      "Wohnzimmer_Licht"};

    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    if (!token || !*token) {
        std::cout << "OPENHAB_TOKEN fehlt – übersprungen\n";
        return 77;  // CTest: übersprungen
    }
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080",
                                  {std::nullopt, std::nullopt, token, 5.0});
    if (!client.isLoggedIn()) {
        std::cout << "NICHT BEREIT: openHAB nicht erreichbar (" << client.lastError()
                  << ")\n";
        return 1;
    }
    namespace ts = openhab::testsuite;
    ts::setLog([](int, const std::string&) {});  // nur unser Bericht zählt
    ts::ItemTester items(client);
    ts::ThingTester things(client);
    ts::RuleTester regeln(client);
    ts::SitemapTester sitemaps(client);
    ts::PersistenceTester persistenz(client);

    int fehler = 0;
    auto melde = [&fehler](const std::string& text) {
        if (!fehler++) std::cout << "NICHT BEREIT:\n";
        std::cout << "  " << text << "\n";
    };
    for (const auto& [name, typ] : ITEMS) {
        if (!items.doesItemExist(name))
            melde("Item fehlt: " + name);
        else if (!items.checkItemIsType(name, typ))
            melde("falscher Typ: " + name);
    }
    for (const auto& uid : THINGS)
        if (!things.isThingOnline(uid)) melde("Thing nicht ONLINE: " + uid);
    for (const auto& uid : REGELN)
        if (!regeln.isRuleIdle(uid)) melde("Regel nicht bereit: " + uid);
    for (const auto& item : AUF_DER_SITEMAP)
        if (!sitemaps.doesSitemapContainItem("restbuch", item))
            melde("nicht auf der Sitemap restbuch: " + item);
    // Ein echter Funktionstest: Kommt ein Command an, und wird der Zustand gemeldet?
    if (!items.testSwitch("Wohnzimmer_Licht", "ON", "ON", 5))
        melde("Command kommt nicht an: Wohnzimmer_Licht");
    // … und landet die Änderung im Persistenzdienst?
    if (!persistenz.isItemPersisted("inmemory", "Wohnzimmer_Licht"))
        melde("nicht in inmemory gespeichert: Wohnzimmer_Licht");

    std::cout << std::string(50, '=') << "\n";
    if (!fehler)
        std::cout << "BEREIT – " << ITEMS.size() << " Items, " << THINGS.size()
                  << " Thing, " << REGELN.size()
                  << " Regel, Sitemap, Persistenz und ein Command\n";
    return fehler ? 1 : 0;
}
