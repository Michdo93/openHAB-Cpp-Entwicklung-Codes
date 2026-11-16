// Kapitel 8: Funktionen, Referenzen, Überladen und Vorgabewerte
#include <iostream>
#include <string>

// Wert, Referenz, konstante Referenz
void verdopple(int wert) {  // bekommt eine Kopie
    wert *= 2;
}
void verdoppleRef(int& wert) {  // bekommt das Original
    wert *= 2;
}
std::string beschrifte(const std::string& name, const std::string& einheit = "%") {
    return name + " [" + einheit + "]";  // const&: keine Kopie, keine Änderung
}

// Überladen: gleicher Name, andere Parameter
std::string zustand(bool an) {
    return an ? "ON" : "OFF";
}
std::string zustand(int prozent) {
    return std::to_string(prozent);
}

// mehrere Ergebnisse: eine Struktur zurückgeben statt Zeiger als Ausgabeparameter
struct Bereich {
    int minimum, maximum;
};
Bereich begrenze(int wert, int schritt) {
    return {wert - schritt, wert + schritt};
}

int zaehler() {
    static int aufrufe = 0;  // lebt so lange wie das Programm
    return ++aufrufe;
}

int main() {
    int x = 10;
    verdopple(x);
    std::cout << "nach verdopple:    " << x << "\n";
    verdoppleRef(x);
    std::cout << "nach verdoppleRef: " << x << "\n";
    std::cout << beschrifte("Dimmer") << ", " << beschrifte("Temperatur", "°C") << "\n";
    std::cout << zustand(true) << " " << zustand(40) << "\n";
    auto [von, bis] = begrenze(50, 10);  // strukturierte Bindung (C++17)
    std::cout << "Bereich " << von << "–" << bis << "\n";
    zaehler();
    zaehler();
    std::cout << "zaehler() wurde " << zaehler() << "-mal aufgerufen\n";
    return 0;
}
