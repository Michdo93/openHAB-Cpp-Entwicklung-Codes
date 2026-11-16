// Kapitel 9: Besitz mit unique_ptr, shared_ptr und weak_ptr
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Sensor {
    std::string name;
    explicit Sensor(std::string n) : name(std::move(n)) {
        std::cout << "  + " << name << "\n";
    }
    ~Sensor() { std::cout << "  - " << name << "\n"; }
};

std::unique_ptr<Sensor> erzeuge(const std::string& name) {
    return std::make_unique<Sensor>(name);  // Besitz geht an den Aufrufer
}

int main() {
    std::cout << "unique_ptr – genau ein Besitzer:\n";
    {
        auto a = erzeuge("Temperatur");
        std::vector<std::unique_ptr<Sensor>> alle;
        alle.push_back(std::move(a));  // Besitz verschieben, nicht kopieren
        alle.push_back(erzeuge("Feuchte"));
        std::cout << "  a ist jetzt " << (a ? "gesetzt" : "leer") << ", " << alle.size()
                  << " im Vektor\n";
    }  // Vektor endet: beide Sensoren frei

    std::cout << "shared_ptr – mehrere Besitzer, Referenzzähler:\n";
    std::weak_ptr<Sensor> beobachter;  // weak_ptr besitzt nicht
    {
        auto s = std::make_shared<Sensor>("Helligkeit");
        auto kopie = s;
        beobachter = s;
        std::cout << "  Besitzer: " << s.use_count() << "\n";
        if (auto p = beobachter.lock()) std::cout << "  lebt noch: " << p->name << "\n";
    }
    std::cout << "  nach dem Block: " << (beobachter.expired() ? "freigegeben" : "lebt")
              << "\n";
    return 0;
}
