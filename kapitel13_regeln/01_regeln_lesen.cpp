// Kapitel 13: Regeln lesen – Übersicht, Module und Skript
#include <openhab/openhab.hpp>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Rules rules(client);
    std::cout << std::left;

    openhab::Rules::GetRulesArgs kurz;
    kurz.summary = true;
    for (const json& r : rules.getRules(kurz))
        std::cout << std::setw(12) << r["uid"].get<std::string>() << " " << std::setw(14)
                  << r["name"].get<std::string>() << " " << std::setw(6)
                  << openhab::str(r, "status.status") << " Tags " << r["tags"].dump()
                  << "\n";

    // Der Aufbau einer Regel: Trigger, Bedingungen, Aktionen – jeweils Module
    const std::string uid = "restbuch-1";
    std::cout << "\nRegel " << uid << ":\n";
    const std::pair<const char*, json> teile[] = {{"Trigger", rules.getTriggers(uid)},
                                                  {"Bedingung", rules.getConditions(uid)},
                                                  {"Aktion", rules.getActions(uid)}};
    for (const auto& [art, module] : teile)
        for (const json& m : module)
            std::cout << "  " << std::setw(10) << art << " " << std::setw(7)
                      << m["id"].get<std::string>() << " " << std::setw(26)
                      << m["type"].get<std::string>() << " "
                      << m["configuration"].dump().substr(0, 36) << "…\n";

    // Ein einzelner Parameter eines Moduls: das Skript der Aktion
    std::string skript =
        rules.getModuleConfigParam(uid, "actions", "script", "script").get<std::string>();
    std::replace(skript.begin(), skript.end(), '\n', '|');  // Zeilen zusammenfassen
    skript.erase(0, skript.find_first_not_of("| "));
    std::cout << "\nSkript der Aktion: " << skript.substr(0, 66) << "\n";
    return 0;
}
