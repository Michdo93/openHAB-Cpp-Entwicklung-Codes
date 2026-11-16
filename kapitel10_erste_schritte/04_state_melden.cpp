// Kapitel 10: States melden
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);

    // Ein Messwert mit Einheit, eine Feuchte und ein Zeitstempel (ISO 8601, UTC)
    std::cout << items.postUpdate("Werkstatt_Temperatur", "18.4 °C") << "\n";
    std::cout << items.postUpdate("Werkstatt_Feuchte", "56 %") << "\n";
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream jetzt;
    jetzt << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    std::cout << items.postUpdate("Werkstatt_LetzteMeldung", jetzt.str()) << "\n";
    // Ein Contact kennt keine Commands – nur States
    std::cout << items.postUpdate("Flur_Haustuer", "OPEN") << "\n";
    // Einheiten werden umgerechnet: Fahrenheit rein, Celsius raus
    std::cout << items.postUpdate("Wohnzimmer_Temperatur", "71.6 °F") << "\n";

    for (const char* name : {"Werkstatt_Temperatur", "Werkstatt_Feuchte", "Flur_Haustuer",
                             "Wohnzimmer_Temperatur"})
        std::cout << std::left << std::setw(22) << name << " = "
                  << items.getItemState(name).get<std::string>() << "\n";
    // Für die weiteren Beispiele wieder 21.5 °C setzen
    items.postUpdate("Wohnzimmer_Temperatur", "21.5 °C");
    return 0;
}
