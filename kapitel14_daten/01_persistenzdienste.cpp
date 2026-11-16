// Kapitel 14: Persistenzdienste und die Items, die sie speichern
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
    openhab::Persistence persistence(client);
    std::cout << std::left;

    for (const json& d : persistence.getServices("de"))
        std::cout << std::setw(9) << d["id"].get<std::string>() << " " << std::setw(10)
                  << d["type"].get<std::string>() << " " << d["label"].get<std::string>()
                  << "\n";

    // Was liegt im Arbeitsspeicher-Dienst – und seit wann?
    json items = persistence.getItemsFromService("inmemory");
    for (size_t i = 0; i < items.size() && i < 6; i++)
        std::cout << "  " << std::setw(24) << items[i]["name"].get<std::string>() << " "
                  << std::right << std::setw(4) << items[i].value("count", 0) << std::left
                  << " Werte, ältester " << openhab::str(items[i], "earliest") << "\n";
    std::cout << "  … " << items.size() << " Items insgesamt\n";

    // rrd4j kann keine Itemliste liefern – die Bibliothek erklärt den Grund
    std::cout << "rrd4j: " << persistence.getItemsFromService("rrd4j") << "\n";
    return 0;
}
