// Kapitel 8: Ausnahmen, std::optional und Ergebniswerte
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

// Ausnahme: für Fehler, die der Aufrufer nicht übersehen soll
double alsZahl(const std::string& text) {
    try {
        return std::stod(text);               // liest "21.5" aus "21.5 °C"
    } catch (const std::invalid_argument&) {  // Ausnahme in eine eigene übersetzen
        throw std::runtime_error("keine Zahl: " + text);
    }
}

// optional: "kein Ergebnis" ist ein normaler Fall
std::optional<int> prozent(const std::string& zustand) {
    if (zustand == "ON") return 100;
    if (zustand == "OFF") return 0;
    try {
        return std::stoi(zustand);
    } catch (const std::exception&) { return std::nullopt; }
}

int main() {
    for (const char* t : {"21.5 °C", "NULL"}) {
        try {
            double wert = alsZahl(t);  // wirft vielleicht
            std::cout << t << " -> " << wert << "\n";
        } catch (const std::exception& e) {
            std::cout << t << " -> Fehler: " << e.what() << "\n";
        }
    }
    for (const char* z : {"ON", "40", "UNDEF"}) {
        std::optional<int> p = prozent(z);
        if (p)
            std::cout << z << " = " << *p << " %\n";
        else
            std::cout << z << " = kein Wert\n";
        std::cout << "  mit Vorgabe: " << p.value_or(-1) << "\n";
    }
    return 0;
}
