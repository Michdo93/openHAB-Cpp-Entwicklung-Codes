// Kapitel 10: Mehrere Anfragen gleichzeitig – ein Client, std::async
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <future>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    std::vector<std::string> namen = {"Wohnzimmer_Licht",     "Wohnzimmer_Dimmer",
                                      "Wohnzimmer_Farblicht", "Wohnzimmer_Temperatur",
                                      "Kueche_Rollladen",     "Flur_Haustuer",
                                      "Haus_Status",          "Werkstatt_Temperatur"};
    // ein Client darf von mehreren Threads gleichzeitig benutzt werden
    auto lesen = [&client](const std::string& item) {
        openhab::json s = openhab::Items(client).getItemState(item);
        return s.is_string() ? s.get<std::string>() : openhab::errorMessage(s);
    };
    using Uhr = std::chrono::steady_clock;

    auto start = Uhr::now();
    std::vector<std::future<std::string>> zukuenfte;
    for (const auto& n : namen)
        zukuenfte.push_back(std::async(std::launch::async, lesen, n));
    std::vector<std::string> states;
    for (auto& z : zukuenfte) states.push_back(z.get());
    auto parallel = Uhr::now() - start;

    start = Uhr::now();
    for (const auto& n : namen) lesen(n);  // zum Vergleich nacheinander
    auto nacheinander = Uhr::now() - start;

    for (size_t i = 0; i < namen.size(); i++)
        std::cout << std::left << std::setw(22) << namen[i] << " = " << states[i] << "\n";
    using std::chrono::duration_cast;
    using ms = std::chrono::milliseconds;
    std::cout << namen.size() << " Anfragen: parallel "
              << duration_cast<ms>(parallel).count() << " ms, nacheinander "
              << duration_cast<ms>(nacheinander).count() << " ms\n";
    return 0;
}
