// Kapitel 8: Klassen, Konstruktor, Destruktor und RAII
#include <iostream>
#include <string>
#include <utility>

class Lampe {
public:
    explicit Lampe(std::string name) : name_(std::move(name)) {
        std::cout << "  + " << name_ << " angelegt\n";
    }
    ~Lampe() { std::cout << "  - " << name_ << " freigegeben\n"; }  // Destruktor

    void schalte(bool an) { an_ = an; }
    bool istAn() const { return an_; }  // const: ändert das Objekt nicht
    const std::string& name() const { return name_; }

private:
    std::string name_;
    bool an_ = false;  // Vorgabewert des Feldes
};

// RAII: eine Ressource gehört einem Objekt und wird im Destruktor freigegeben
class Verbindung {
public:
    explicit Verbindung(const std::string& ziel) : ziel_(ziel) {
        std::cout << "  Verbindung zu " << ziel_ << " geöffnet\n";
    }
    ~Verbindung() { std::cout << "  Verbindung zu " << ziel_ << " geschlossen\n"; }
    Verbindung(const Verbindung&) = delete;  // nicht kopierbar
    Verbindung& operator=(const Verbindung&) = delete;

private:
    std::string ziel_;
};

int main() {
    std::cout << "Block beginnt\n";
    {
        Lampe decke("Deckenlicht");
        Verbindung v("openHAB");
        decke.schalte(true);
        std::cout << "  " << decke.name() << ": " << (decke.istAn() ? "ON" : "OFF") << "\n";
    }  // hier enden beide Objekte
    std::cout << "Block beendet\n";
    return 0;
}
