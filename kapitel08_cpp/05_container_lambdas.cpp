// Kapitel 8: vector, map, Lambdas und Algorithmen
#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

struct Item {
    std::string name;
    std::string typ;
    double wert;
};

int main() {
    std::vector<Item> items = {{"Wohnzimmer_Temperatur", "Number", 21.5},
                               {"Kueche_Temperatur", "Number", 19.0},
                               {"Wohnzimmer_Dimmer", "Dimmer", 40},
                               {"Bad_Temperatur", "Number", 23.0}};
    items.push_back({"Flur_Dimmer", "Dimmer", 0});

    // Lambda als Vergleich: nach Wert absteigend sortieren
    std::sort(items.begin(), items.end(),
              [](const Item& a, const Item& b) { return a.wert > b.wert; });
    for (const auto& i : items) std::cout << i.name << " = " << i.wert << "\n";

    // Lambda mit Erfassung: zählen, was über einer Grenze liegt
    double grenze = 20.0;
    auto warm = std::count_if(items.begin(), items.end(), [grenze](const Item& i) {
        return i.typ == "Number" && i.wert > grenze;
    });
    std::cout << warm << " Räume wärmer als " << grenze << " °C\n";

    // map: nach Typ gruppieren, Schlüssel sortiert
    std::map<std::string, std::vector<std::string>> nachTyp;
    for (const auto& i : items) nachTyp[i.typ].push_back(i.name);
    for (const auto& [typ, namen] : nachTyp)
        std::cout << typ << ": " << namen.size() << "\n";

    // Mittelwert mit accumulate
    double summe = std::accumulate(items.begin(), items.end(), 0.0,
                                   [](double s, const Item& i) { return s + i.wert; });
    std::cout << "Mittelwert: " << summe / items.size() << "\n";

    // suchen
    auto it = std::find_if(items.begin(), items.end(),
                           [](const Item& i) { return i.name == "Flur_Dimmer"; });
    if (it != items.end()) std::cout << "gefunden: " << it->name << "\n";
    return 0;
}
