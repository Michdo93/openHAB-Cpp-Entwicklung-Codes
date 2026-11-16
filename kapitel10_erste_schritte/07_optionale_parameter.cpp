// Kapitel 10: Optionale Parameter – Vorgaben, Argument-Strukturen und JSON-Pfade
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iostream>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);

    // 1. nur Pflichtparameter: Alle optionalen haben die Vorgabe der Python-Bibliothek
    std::cout << "alle Items:    " << items.getItems().size() << "\n";

    // 2. die Argument-Struktur: nur setzen, was gebraucht wird
    openhab::Items::GetItemsArgs a;
    a.fields = "name";
    a.tags = "Light";
    std::cout << "Licht, Felder: " << items.getItems(a).dump() << "\n";

    // 3. positionell in der Reihenfolge der Python-Bibliothek
    json mit = items.getItem("gLichter");                // recursive: Vorgabe true
    json ohne = items.getItem("gLichter", ".*", false);  // metadata, recursive
    std::cout << "gLichter:      " << mit["members"].size() << " Mitglieder (recursive), "
              << ohne["members"].size() << " ohne, Funktion "
              << openhab::str(mit, "function.name") << "\n";

    // 4. std::nullopt überspringt einen optionalen Parameter
    json schalter = items.getItems("Switch", std::nullopt, ".*", false, "name");
    std::cout << "Switch-Items:  " << schalter.size() << "\n";

    // JSON-Pfade: Punkte trennen Felder, Zahlen sind Array-Positionen
    json item = items.getItem("Wohnzimmer_Temperatur");
    std::cout << "Muster:        " << openhab::str(item, "stateDescription.pattern")
              << "\n";
    std::cout << "erstes Tag:    " << openhab::str(item, "tags.0") << "\n";
    bool fehlt = openhab::at(item, "metadata.unit").is_null();
    std::cout << "fehlt:         " << (fehlt ? "(null)" : "da") << "\n";
    std::cout << "Zahl/Vorgabe:  " << openhab::num(item, "stateDescription.step", -1)
              << "\n";
    // nlohmann/json selbst kennt JSON-Pointer: "/feld/feld" mit Vorgabewert
    bool nurLesen = item.value("/stateDescription/readOnly"_json_pointer, true);
    std::cout << "JSON-Pointer:  readOnly = " << std::boolalpha << nurLesen << "\n";
    return 0;
}
