// Kapitel 18: Messwerte des Rechners
#include "messwerte.hpp"

#include <unistd.h>

#include <fstream>
#include <sstream>
#include <thread>

namespace messwerte {

std::optional<double> temperaturAus(std::string_view text) {
    std::istringstream ein{std::string(text)};
    long milli;
    if (!(ein >> milli)) return std::nullopt;
    return milli / 1000.0;
}

std::optional<double> lastAus(std::string_view loadavg, int kerne) {
    std::istringstream ein{std::string(loadavg)};
    double last;
    if (!(ein >> last) || kerne < 1) return std::nullopt;
    return last / kerne * 100;
}

std::optional<double> speicherAus(std::string_view meminfo) {
    std::istringstream ein{std::string(meminfo)};
    std::string schluessel;
    long wert;
    std::optional<long> gesamt, frei;
    while (ein >> schluessel >> wert) {  // "MemTotal:  8000000 kB"
        if (schluessel == "MemTotal:") gesamt = wert;
        if (schluessel == "MemAvailable:") frei = wert;
        ein.ignore(1000, '\n');
    }
    if (!gesamt || !frei || *gesamt <= 0) return std::nullopt;
    return 100.0 * (*gesamt - *frei) / *gesamt;
}

std::optional<long long> netzbytesAus(std::string_view netdev) {
    std::istringstream ein{std::string(netdev)};
    std::string zeile;
    std::getline(ein, zeile);  // zwei Kopfzeilen überspringen
    if (!std::getline(ein, zeile)) return std::nullopt;
    long long summe = 0;
    // "  eth0: rx_bytes packets errs drop fifo frame compressed multicast tx_bytes …"
    while (std::getline(ein, zeile)) {
        auto doppelpunkt = zeile.find(':');
        if (doppelpunkt == std::string::npos) continue;
        std::istringstream felder(zeile.substr(doppelpunkt + 1));
        std::string name = zeile.substr(0, doppelpunkt);
        name.erase(0, name.find_first_not_of(' '));
        long long werte[9];
        for (long long& w : werte) felder >> w;
        if (felder && name != "lo") summe += werte[0] + werte[8];
    }
    return summe;
}

std::optional<std::string> datei(const std::string& pfad) {
    std::ifstream f(pfad);
    if (!f) return std::nullopt;
    std::ostringstream inhalt;
    inhalt << f.rdbuf();
    return inhalt.str();
}

}  // namespace messwerte

Messung Messwerte::lesen() {
    using namespace messwerte;
    Messung m;
    if (auto t = datei("/sys/class/thermal/thermal_zone0/temp"))
        m.temperatur = temperaturAus(*t);
    if (auto l = datei("/proc/loadavg"))
        m.last = lastAus(*l, static_cast<int>(std::thread::hardware_concurrency()));
    if (auto s = datei("/proc/meminfo")) m.speicher = speicherAus(*s);
    if (auto u = datei("/proc/uptime")) m.laufzeitH = std::stod(*u) / 3600;

    auto jetzt = std::chrono::steady_clock::now();
    std::optional<long long> bytes;
    if (auto n = datei("/proc/net/dev")) bytes = netzbytesAus(*n);
    if (bytes && vorherBytes_ && jetzt > vorherZeit_) {
        std::chrono::duration<double> sekunden = jetzt - vorherZeit_;
        m.netzKbit = (*bytes - *vorherBytes_) * 8 / 1000.0 / sekunden.count();
    }
    vorherBytes_ = bytes;
    vorherZeit_ = jetzt;
    return m;
}
