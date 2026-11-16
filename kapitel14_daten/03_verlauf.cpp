// Kapitel 14: Historische Werte abfragen, auswerten und im Terminal zeichnen
#include <openhab/openhab.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

using openhab::json;
const int HOEHE = 8;  // Zeilen des Diagramms

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
    std::time_t ende = std::time(nullptr), start = ende - 24 * 3600;

    openhab::Persistence::GetItemPersistenceDataArgs a;  // Testdaten aus 02_testdaten
    a.serviceID = "inmemory";
    a.startTime = iso(start);
    a.endTime = iso(ende);
    json antwort = persistence.getItemPersistenceData("Wohnzimmer_Temperatur", a);
    const json daten = antwort.value("data", json::array());
    if (daten.empty()) {
        std::cerr << "keine Werte – zuerst 02_testdaten ausführen\n";
        return 1;
    }
    std::vector<double> werte;
    for (const json& p : daten) werte.push_back(std::stod(p["state"].get<std::string>()));
    auto [kleinster, groesster] = std::minmax_element(werte.begin(), werte.end());
    double min = *kleinster, max = *groesster;
    double mittel = std::accumulate(werte.begin(), werte.end(), 0.0) / werte.size();
    std::time_t warm = daten[groesster - werte.begin()]["time"].get<long long>() / 1000;

    std::cout << antwort["datapoints"].get<std::string>()
              << " Messwerte der letzten 24 Stunden (" << antwort["unit"].get<std::string>()
              << ")\n"
              << std::fixed << std::setprecision(1) << "Minimum " << min << " °C   Maximum "
              << max << " °C   Mittel " << mittel << " °C\n"
              << "am wärmsten um " << std::put_time(std::localtime(&warm), "%H:%M")
              << " Uhr\n\n";

    // Säulendiagramm: eine Spalte je Stunde, HOEHE Zeilen von oben nach unten
    for (int zeile = HOEHE; zeile >= 1; zeile--) {
        double schwelle = min + (max - min) * (zeile - 0.5) / HOEHE;
        if (zeile == HOEHE || zeile == 1)
            std::cout << std::setw(5) << (zeile == HOEHE ? max : min) << " |";
        else
            std::cout << "      |";
        for (size_t i = 0; i < werte.size(); i += 4)
            std::cout << (werte[i] >= schwelle ? '#' : ' ');
        std::cout << "\n";
    }
    std::cout << "      +";
    for (size_t i = 0; i < werte.size(); i += 4) std::cout << (i % 24 == 0 ? '+' : '-');
    std::cout << "\n       vor 24 h" << std::string(werte.size() / 4 - 13, ' ')
              << "jetzt\n";

    // Seitenweise abrufen: 10 Werte pro Seite – sofern der Dienst Seiten unterstützt
    a.page = 0;
    a.pageLength = 10;
    json seite = persistence.getItemPersistenceData("Wohnzimmer_Temperatur", a);
    std::cout << "\nSeite 0: " << openhab::str(seite, "data.0.state") << ", "
              << openhab::str(seite, "data.1.state") << ", "
              << openhab::str(seite, "data.2.state") << " …\n";

    // Mit boundary liefert openHAB zusätzlich den Wert vor und nach dem Zeitraum
    openhab::Persistence::GetItemPersistenceDataArgs rand;
    rand.serviceID = "inmemory";
    rand.startTime = iso(ende - 300);
    rand.endTime = iso(ende);
    rand.boundary = true;
    json r = persistence.getItemPersistenceData("Wohnzimmer_Temperatur", rand);
    std::cout << "letzte 5 Minuten mit Rand: " << r["datapoints"].get<std::string>()
              << " Wert(e)\n";
    return 0;
}
