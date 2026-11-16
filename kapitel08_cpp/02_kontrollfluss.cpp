// Kapitel 8: if, switch, Schleifen und range-for mit Item-Zuständen
#include <iostream>
#include <string>
#include <vector>

enum class Befehl { An, Aus, Heller, Unbekannt };

Befehl erkenne(const std::string& text) {
    if (text == "ON") return Befehl::An;  // == vergleicht den Inhalt
    if (text == "OFF") return Befehl::Aus;
    if (text == "INCREASE") return Befehl::Heller;
    return Befehl::Unbekannt;
}

int main() {
    std::vector<std::string> zustaende = {"ON", "OFF", "INCREASE", "TOGGLE"};
    for (const std::string& z : zustaende) {  // range-for über alle Elemente
        switch (erkenne(z)) {
        case Befehl::An:
            std::cout << z << ": Licht an\n";
            break;
        case Befehl::Aus:
            std::cout << z << ": Licht aus\n";
            break;
        case Befehl::Heller:
            std::cout << z << ": heller\n";
            break;
        default:
            std::cout << z << ": unbekannter Befehl\n";
        }
    }

    // klassische Schleife: Dimmer in Zehnerschritten
    for (int wert = 0; wert <= 30; wert += 10) std::cout << "Dimmer " << wert << " %\n";

    // while mit Abbruchbedingung
    double temperatur = 17.0;
    int minuten = 0;
    while (temperatur < 20.0) {
        temperatur += 0.8;
        minuten += 5;
    }
    std::cout << "20 °C nach " << minuten << " Minuten (" << temperatur << " °C)\n";

    // if mit Initialisierung (C++17)
    if (auto n = zustaende.size(); n > 3) std::cout << n << " Zustände geprüft\n";
    return 0;
}
