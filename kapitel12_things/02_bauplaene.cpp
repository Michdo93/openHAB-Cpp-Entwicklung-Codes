// Kapitel 12: Baupläne – Thing-Typen, Channel-Gruppen, Konfiguration, Channel-Typen
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    std::cout << std::left;

    for (const json& t : openhab::ThingTypes(client).getThingTypes("astro", "de"))
        std::cout << std::setw(12) << t["UID"].get<std::string>() << " "
                  << t["label"].get<std::string>() << "\n";

    json mond = openhab::ThingTypes(client).getThingType("astro:moon", "de");
    std::cout << "\n" << mond["label"].get<std::string>() << " – Channel-Gruppen:\n";
    for (const json& g : mond["channelGroups"]) {
        const json& channels = g["channels"];
        std::cout << "  " << std::setw(10) << g["id"].get<std::string>();
        for (size_t i = 0; i < channels.size() && i < 5; i++)
            std::cout << (i ? ", " : " ") << channels[i]["id"].get<std::string>();
        if (channels.size() > 5) std::cout << " … (" << channels.size() << " insgesamt)";
        std::cout << "\n";
    }

    // Welche Konfiguration braucht ein Mond-Thing?
    json beschreibung = openhab::ConfigDescriptions(client).getConfigDescription(
        "thing-type:astro:moonconfig", "de");
    std::cout << "\nKonfiguration:\n";
    for (const json& p : beschreibung["parameters"]) {
        std::string pflicht = p.value("required", false)
                                  ? "Pflicht"
                                  : "Standard " + p.value("default", std::string("–"));
        std::cout << "  " << std::setw(12) << p["name"].get<std::string>() << " "
                  << std::setw(8) << p["type"].get<std::string>() << " " << std::setw(17)
                  << pflicht << " " << p["label"].get<std::string>() << "\n";
    }

    // Ein Channel-Typ verrät, zu welchem Item-Typ er passt
    json kanal = openhab::ChannelTypes(client).getChannelType("astro:phaseName", "de");
    std::cout << "\nChannel-Typ " << kanal["UID"].get<std::string>() << ": "
              << kanal["label"].get<std::string>() << ", Item-Typ "
              << kanal["itemType"].get<std::string>() << "\nOptionen:";
    json optionen = openhab::at(kanal, "stateDescription.options");
    for (size_t i = 0; i < optionen.size() && i < 5; i++)
        std::cout << (i ? ", " : " ") << optionen[i]["label"].get<std::string>();
    std::cout << " …\n";
    return 0;
}
