// Kapitel 12: Welche Add-ons gibt es, welche sind installiert?
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Addons addons(client);

    json alle = addons.getAddons(std::nullopt, "de");  // serviceID, language
    int installiert = 0;
    for (const json& a : alle) installiert += a.value("installed", false);
    std::cout << alle.size() << " Add-ons verfügbar, " << installiert << " installiert:\n";
    for (const json& a : alle)
        if (a.value("installed", false))
            std::cout << "  " << std::left << std::setw(24) << a["uid"].get<std::string>()
                      << " " << std::setw(8) << a["version"].get<std::string>() << " "
                      << a["label"].get<std::string>() << "\n";

    // Wie viele gibt es von jeder Art? Eine map zählt mit.
    std::map<std::string, int> nachArt;
    for (const json& a : alle) nachArt[a["type"].get<std::string>()]++;
    std::cout << "\nVerfügbar nach Art:";
    for (const auto& [art, anzahl] : nachArt) std::cout << " " << art << "=" << anzahl;
    std::cout << "\n";

    json astro = addons.getAddon("binding-astro", std::nullopt, "de");
    std::string beschreibung = astro["description"];
    std::cout << "\n"
              << astro["label"].get<std::string>() << ": " << beschreibung.substr(0, 70)
              << "…\n";
    std::cout << "Logger-Pakete: " << astro["loggerPackages"].dump() << "\n";

    std::cout << "Installationsquellen:";
    for (const json& q : addons.getAddonServices())
        std::cout << " " << q["id"].get<std::string>();
    std::cout << "\n";
    return 0;
}
