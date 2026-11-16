// Kapitel 18: Messwerte des Rechners aus /proc und /sys.
// Die Zerlege-Funktionen arbeiten auf Text – so lassen sie sich ohne Rechner testen.
// Ein fehlender Wert ist std::nullopt statt einer Zauberzahl wie -999.
#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

struct Messung {
    std::optional<double> temperatur;  // °C
    std::optional<double> last;        // Last der letzten Minute je Kern in %
    std::optional<double> speicher;    // belegter Arbeitsspeicher in %
    std::optional<double> netzKbit;    // Netzverkehr seit der letzten Messung
    std::optional<double> laufzeitH;   // Laufzeit des Rechners in Stunden
};

namespace messwerte {
std::optional<double> temperaturAus(std::string_view text);          // "48312\n" → 48.312
std::optional<double> lastAus(std::string_view loadavg, int kerne);  // "0.52 …" → 13 (%)
std::optional<double> speicherAus(std::string_view meminfo);     // MemTotal/MemAvailable
std::optional<long long> netzbytesAus(std::string_view netdev);  // rx+tx ohne lo
std::optional<std::string> datei(const std::string& pfad);       // ganze Datei als Text
}  // namespace messwerte

// Liest alles auf einmal und merkt sich die letzte Netzmessung für die Rate
class Messwerte {
public:
    Messung lesen();

private:
    std::optional<long long> vorherBytes_;
    std::chrono::steady_clock::time_point vorherZeit_;
};
