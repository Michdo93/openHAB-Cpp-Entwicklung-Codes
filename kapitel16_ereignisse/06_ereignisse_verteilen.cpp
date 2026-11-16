// Kapitel 16: Ein Strom für das ganze Programm – Ereignisse an Beobachter verteilen
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// Der Verteiler (Subject im Observer-Muster): ein Strom, beliebig viele Beobachter
class Verteiler {
public:
    using Beobachter = std::function<void(const std::string& item, const openhab::json&)>;

    explicit Verteiler(openhab::EventStream strom) : strom_(std::move(strom)) {}
    ~Verteiler() { stoppen(); }

    void anmelden(std::string item, Beobachter fn) {  // "*" für alle Items
        liste_.emplace_back(std::move(item), std::move(fn));
    }
    void starten() {
        leser_ = std::thread([this] { lesen(); });
    }
    void stoppen() {
        strom_.interrupt();  // Leser aufwecken und beenden
        if (leser_.joinable()) leser_.join();
    }

private:
    void lesen() {  // der einzige Leser des Stroms
        for (const openhab::json& e : strom_) {
            std::string topic = e["topic"];
            std::string name = topic.substr(14, topic.find('/', 14) - 14);
            for (const auto& [item, fn] : liste_)
                if (item == name || item == "*") fn(name, e["payload"]);
        }
    }
    openhab::EventStream strom_;
    std::vector<std::pair<std::string, Beobachter>> liste_;
    std::thread leser_;
};

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");

    Verteiler v(openhab::ItemEvents(client).ItemStateChangedEvent());
    auto anzeigen = [](std::string beschriftung) {  // Lambda, das ein Lambda liefert
        return [beschriftung](const std::string&, const openhab::json& p) {
            std::cout << std::left << std::setw(17) << beschriftung
                      << openhab::str(p, "oldValue") << " -> " << openhab::str(p, "value")
                      << "\n";
        };
    };
    int anzahl = 0;
    v.anmelden("Wohnzimmer_Licht", anzeigen("Deckenlicht"));
    v.anmelden("Wohnzimmer_Dimmer", anzeigen("Leselampe"));
    v.anmelden("Wohnzimmer_Dimmer", anzeigen("Leselampe (2)"));  // derselbe Strom
    v.anmelden("*", [&anzahl](const std::string&, const openhab::json&) { anzahl++; });

    v.starten();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    openhab::Items(client).sendCommand("Wohnzimmer_Licht", "OFF");
    openhab::Items(client).sendCommand("Wohnzimmer_Dimmer", "45");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    v.stoppen();
    std::cout << anzahl << " Ereignisse über einen einzigen Strom verteilt\n";
    return 0;
}
