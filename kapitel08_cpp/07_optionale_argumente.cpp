// Kapitel 8: Vorgabewerte, std::nullopt und Argument-Strukturen
#include <iostream>
#include <optional>
#include <string>

// So sieht eine Methode der Bibliothek aus: Pflichtparameter zuerst, dann die
// optionalen mit den Vorgabewerten der Python-Bibliothek.
std::string suche(const std::string& typ,
                  const std::optional<std::string>& tags = std::nullopt,
                  const std::string& metadata = ".*", bool recursive = false) {
    std::string t = "typ=" + typ + " metadata=" + metadata;
    if (tags) t += " tags=" + *tags;  // nur gesetzt, wenn angegeben
    return t + (recursive ? " recursive" : "");
}

// ... und die Struktur mit denselben optionalen Parametern
struct SucheArgs {
    std::optional<std::string> tags;
    std::string metadata = ".*";
    bool recursive = false;
};
std::string suche(const std::string& typ, const SucheArgs& a) {
    return suche(typ, a.tags, a.metadata, a.recursive);
}

int main() {
    std::cout << suche("Switch") << "\n";                             // alle Vorgaben
    std::cout << suche("Dimmer", "Light") << "\n";                    // der Reihe nach
    std::cout << suche("Number", std::nullopt, "semantics") << "\n";  // einen überspringen

    SucheArgs a;  // C++17: Struktur
    a.recursive = true;
    std::cout << suche("Group", a) << "\n";
#if __cplusplus >= 202002L
    std::cout << suche("Color", {.tags = "Light", .recursive = true}) << "\n";  // C++20
#endif
    return 0;
}
