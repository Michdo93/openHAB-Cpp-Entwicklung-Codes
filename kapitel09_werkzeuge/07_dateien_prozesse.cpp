// Kapitel 9: Dateien, std::filesystem und Prozesse – Messwerte aus /proc
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

int main() {
    // /proc/loadavg: "0.52 0.48 0.40 2/931 12345" – die Systemlast der letzten Minute
    double last = -1;
    std::ifstream("/proc/loadavg") >> last;
    std::cout << std::fixed << std::setprecision(2) << "Systemlast:  " << last << "\n";

    // /proc/meminfo zeilenweise lesen
    long gesamt = 0, frei = 0;
    std::ifstream meminfo("/proc/meminfo");
    for (std::string zeile; std::getline(meminfo, zeile);) {
        std::istringstream s(zeile);
        std::string name;
        long kb = 0;
        s >> name >> kb;
        if (name == "MemTotal:") gesamt = kb;
        if (name == "MemAvailable:") frei = kb;
    }
    std::cout << "Speicher:    " << frei / 1024 << " von " << gesamt / 1024
              << " MiB frei\n";

    // eine Datei schreiben; der Destruktor schließt sie
    fs::path datei = fs::temp_directory_path() / "messwerte.csv";
    {
        std::ofstream out(datei);
        out << "messwert;wert\nlast;" << last << "\nfrei_mib;" << frei / 1024 << "\n";
        if (!out) return 1;
    }
    std::cout << "Datei:       " << datei.filename().string() << " mit "
              << fs::file_size(datei) << " Byte\n";
    fs::remove(datei);

    // popen startet ein Programm und liest seine Ausgabe
    struct Schliessen {
        void operator()(FILE* f) const { pclose(f); }
    };
    std::unique_ptr<FILE, Schliessen> p(popen("uname -s", "r"));
    char zeile[128] = "";
    if (p && std::fgets(zeile, sizeof zeile, p.get()))
        std::cout << "System:      " << zeile;
    return 0;
}
