// Kapitel 14: Werte mit eigenem Zeitstempel in einen Persistenzdienst schreiben
// 48 Stunden synthetische Temperaturen (alle 15 Minuten) für den Dienst "inmemory" –
// einschließlich eines Ausreißers.
#include <openhab/openhab.hpp>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

const std::string DIENST = "inmemory";

std::string iso(std::time_t t) {
    std::ostringstream s;
    s << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return s.str();
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Persistence persistence(client);
    std::mt19937 zufall(42);  // fester Startwert: jeder Lauf erzeugt dieselben Daten
    std::normal_distribution<double> rauschen(0.0, 0.15);
    std::time_t jetzt = std::time(nullptr) / 60 * 60;
    const std::pair<const char*, double> items[] = {{"Wohnzimmer_Temperatur", 21.0},
                                                    {"Werkstatt_Temperatur", 17.0}};

    for (const auto& [item, grund] : items) {
        // alte Testdaten der letzten drei Tage entfernen
        persistence.deleteItemData(item, iso(jetzt - 3 * 86400), iso(jetzt), DIENST);
        int geschrieben = 0;
        for (int k = 48 * 4; k > 0; k--) {
            std::time_t t = jetzt - 15 * 60 * k;
            std::tm lokal = *std::localtime(&t);
            double stunde = lokal.tm_hour + lokal.tm_min / 60.0;
            double wert =
                grund + 1.5 * std::sin((stunde - 9) / 24 * 2 * M_PI) + rauschen(zufall);
            if (grund == 17.0 && k == 30) wert += 6.0;  // ein Ausreißer
            std::ostringstream zustand;
            zustand << std::fixed << std::setprecision(2) << wert << " °C";
            openhab::json r =
                persistence.storeItemData(item, iso(t), zustand.str(), DIENST);
            if (openhab::isError(r)) {
                std::cerr << item << ": " << openhab::errorMessage(r) << "\n";
                return 1;
            }
            geschrieben++;
        }
        std::cout << item << ": " << geschrieben << " Werte in »" << DIENST
                  << "« geschrieben\n";
    }

    // Gegenprobe: rrd4j nimmt keine Werte von außen an
    std::string fehler = openhab::errorMessage(
        persistence.storeItemData(items[0].first, iso(jetzt), "20 °C", "rrd4j"));
    std::cout << "Gegenprobe rrd4j: " << fehler.substr(fehler.rfind('(')) << "\n";
    return 0;
}
