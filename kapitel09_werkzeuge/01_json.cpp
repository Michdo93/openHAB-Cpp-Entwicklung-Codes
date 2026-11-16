// Kapitel 9: JSON mit nlohmann/json – lesen, durchlaufen, bauen, ausgeben
#include <nlohmann/json.hpp>

#include <iostream>

using json = nlohmann::ordered_json;  // ordered: Schlüssel in ihrer Reihenfolge

int main() {
    // So antwortet openHAB auf GET /rest/items/Wohnzimmer_Dimmer
    std::string text = R"({"type":"Dimmer","name":"Wohnzimmer_Dimmer","state":"40",
        "label":"Leselampe","tags":["Light"],"groupNames":["gLichter","gWohnzimmer"]})";
    json item = json::parse(text);

    std::cout << "Name:  " << item["name"].get<std::string>() << "\n";
    int prozent = std::stoi(item["state"].get<std::string>());  // Zustände sind Text
    std::cout << "State: " << prozent << " %\n";
    std::cout << "Tags:  " << item["tags"].size() << ", erste: " << item["tags"][0] << "\n";
    std::cout << "Icon:  " << item.value("category", "(keins)") << "\n";  // mit Vorgabe
    std::cout << "hat label: " << std::boolalpha << item.contains("label") << "\n";

    for (const auto& gruppe : item["groupNames"])
        std::cout << "  Gruppe " << gruppe << "\n";
    for (const auto& [schluessel, wert] : item.items())
        if (wert.is_string()) std::cout << "  " << schluessel << " = " << wert << "\n";

    // Ein Item zum Anlegen bauen – Initialisierungslisten werden zu Objekten und Arrays
    json neu = {{"type", "Switch"},
                {"name", "Flur_Licht"},
                {"label", "Flurlicht"},
                {"tags", {"Light"}},
                {"groupNames", json::array()}};
    neu["category"] = "lightbulb";
    std::cout << neu.dump() << "\n";
    std::cout << neu.dump(2) << "\n";

    // Fehler beim Lesen: ohne Ausnahme mit parse(…, nullptr, false)
    json kaputt = json::parse("{\"name\": ", nullptr, false);
    std::cout << "kaputt ist verworfen: " << kaputt.is_discarded() << "\n";
    return 0;
}
