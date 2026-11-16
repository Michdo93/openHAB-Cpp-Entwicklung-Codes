// Kapitel 8: Werte, Typen, auto und Ausgabe
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>

int main() {
    int helligkeit = 67;       // ganze Zahl
    double temperatur = 21.5;  // Gleitkommazahl
    bool eingeschaltet = true;
    std::string name = "Wohnzimmer_Licht";  // Zeichenkette mit eigener Länge
    auto einheit = std::string("°C");       // auto: Typ aus dem Initialisierer
    std::uint8_t prozent = 255;             // genau ein Byte ohne Vorzeichen

    std::cout << name << ": " << (eingeschaltet ? "ON" : "OFF") << ", " << helligkeit
              << " %\n";
    std::cout << std::fixed << std::setprecision(1) << "Temperatur: " << temperatur << " "
              << einheit << "\n";
    std::cout << "Größen: int " << sizeof(int) << ", double " << sizeof(double)
              << ", string " << sizeof(std::string) << " Byte\n";
    std::cout << "Länge von \"" << einheit << "\": " << einheit.size() << " Byte\n";
    std::cout << "67 / 3 = " << helligkeit / 3 << ", 67 / 3.0 = " << helligkeit / 3.0
              << "\n";
    prozent++;  // läuft still über
    std::cout << "255 + 1 als uint8_t: " << static_cast<int>(prozent) << "\n";

    const double sollwert = 21.0;       // const: nicht mehr änderbar
    constexpr int maxHelligkeit = 100;  // schon beim Übersetzen bekannt
    std::cout << "Abweichung: " << std::showpos << temperatur - sollwert << std::noshowpos
              << " K, Maximum " << maxHelligkeit << "\n";
    return 0;
}
