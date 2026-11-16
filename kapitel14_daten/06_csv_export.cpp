// Kapitel 14: Den Verlauf eines Items als CSV-Datei exportieren – in Zeitabschnitten
//   ./starte.sh kapitel14_daten/06_csv_export.cpp [Item] [Stunden] [Dienst]
#include <openhab/openhab.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using openhab::json;

std::string iso(std::time_t t) {
    std::ostringstream s;
    s << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return s.str();
}

int main(int argc, char** argv) {
    std::string item = argc > 1 ? argv[1] : "Wohnzimmer_Temperatur";
    int stunden = argc > 2 ? std::atoi(argv[2]) : 48;
    std::string dienst = argc > 3 ? argv[3] : "inmemory";
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Persistence persistence(client);

    std::string datei = item + ".csv";
    std::ofstream csv(datei);
    csv << "zeit;wert;einheit\n";

    // Lange Zeiträume in Abschnitten von 6 Stunden abfragen: kleine Antworten
    const int abschnitt = 6 * 3600;
    std::time_t ende = std::time(nullptr);
    int zeilen = 0;
    for (std::time_t von = ende - stunden * 3600; von < ende; von += abschnitt) {
        openhab::Persistence::GetItemPersistenceDataArgs a;
        a.serviceID = dienst;
        a.startTime = iso(von);
        a.endTime = iso(std::min(von + abschnitt, ende) - 1);  // Grenzen nicht doppelt
        json antwort = persistence.getItemPersistenceData(item, a);
        if (openhab::isError(antwort)) {
            std::cerr << openhab::errorMessage(antwort) << "\n";
            return 1;
        }
        std::string einheit = openhab::str(antwort, "unit");
        for (const json& p : antwort["data"]) {
            std::string wert = p["state"];
            std::replace(wert.begin(), wert.end(), '.', ',');  // Dezimalkomma für Tabellen
            csv << iso(p["time"].get<long long>() / 1000) << ";" << wert << ";" << einheit
                << "\n";
            zeilen++;
        }
    }
    csv.close();
    if (!csv) {
        std::cerr << datei << ": Schreibfehler\n";
        return 1;
    }
    std::cout << zeilen << " Werte (" << stunden << " h, " << dienst << ") nach " << datei
              << " geschrieben\n";
    return 0;
}
