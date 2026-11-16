// Kapitel 13: Welche Bausteine gibt es für Regeln?
#include <openhab/openhab.hpp>

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::ModuleTypes moduleTypes(client);
    std::cout << std::left;

    json alle = moduleTypes.getModuleTypes(std::nullopt, std::nullopt, "de");
    const char* arten[] = {"Trigger", "Bedingung", "Aktion"};
    for (int art = 0; art < 3; art++) {
        std::vector<json> liste;
        for (const json& m : alle) {
            std::string uid = m["uid"];
            if (m["visibility"] != "VISIBLE" || uid.find('#') != std::string::npos)
                continue;  // ohne Thing-Actions
            int a = uid.find("Trigger") != std::string::npos     ? 0
                    : uid.find("Condition") != std::string::npos ? 1
                                                                 : 2;
            if (a == art) liste.push_back(m);
        }
        std::sort(liste.begin(), liste.end(),
                  [](const json& a, const json& b) { return a["uid"] < b["uid"]; });
        std::cout << arten[art] << " (" << liste.size() << "):\n";
        for (size_t i = 0; i < liste.size() && i < 7; i++)
            std::cout << "  " << std::setw(30) << liste[i]["uid"].get<std::string>() << " "
                      << liste[i]["label"].get<std::string>() << "\n";
    }

    json typ = moduleTypes.getModuleType("core.ItemCommandAction", "de");
    std::cout << "\n"
              << typ["uid"].get<std::string>() << " – "
              << typ["description"].get<std::string>() << "\n";
    for (const json& p : typ["configDescriptions"])
        std::cout << "  " << std::setw(10) << p["name"].get<std::string>() << " "
                  << std::setw(6) << p["type"].get<std::string>() << " " << std::setw(8)
                  << (p.value("required", false) ? "Pflicht" : "optional") << " "
                  << p["label"].get<std::string>() << "\n";

    std::cout << "\nRegelvorlagen installiert: "
              << openhab::Templates(client).getTemplates("de").size() << "\n";
    return 0;
}
