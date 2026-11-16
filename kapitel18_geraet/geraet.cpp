// Kapitel 18: Die Logik des Geräts
#include "geraet.hpp"

#include <sys/statvfs.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

std::string mitEinheit(double wert, int stellen, const std::string& einheit) {
    std::ostringstream text;
    text << std::fixed << std::setprecision(stellen) << wert << " " << einheit;
    return text.str();
}

namespace {
struct ItemDef {
    const char *name, *typ, *label, *einheit, *muster;
};
// Die Items des Geräts: Name, Typ, Label, Einheit (oder nullptr), Anzeigeformat
const std::vector<ItemDef> ITEMS = {
    {"Pi_Temperatur", "Number:Temperature", "CPU-Temperatur", "°C", "%.1f °C"},
    {"Pi_Last", "Number:Dimensionless", "Last", "%", "%.0f %%"},
    {"Pi_Speicher", "Number:Dimensionless", "Speicher belegt", "%", "%.0f %%"},
    {"Pi_Netz", "Number:DataTransferRate", "Netzverkehr", "kbit/s", "%.1f kbit/s"},
    {"Pi_Laufzeit", "Number:Time", "Laufzeit", "h", "%.1f h"},
    {"Pi_Luefter", "Switch", "Lüfter", nullptr, nullptr},
    {"Pi_Online", "Switch", "Gerät erreichbar", nullptr, nullptr},
    {"Pi_Aktion", "String", "Aktion", nullptr, nullptr},
    {"Pi_Antwort", "String", "Antwort", nullptr, nullptr},
};
}  // namespace

void Geraet::melde(const std::string& item, const std::string& state) {
    openhab::json r = openhab::Items(client_).postUpdate(item, state);
    if (openhab::isError(r)) std::cerr << item << ": " << openhab::errorMessage(r) << "\n";
}

bool Geraet::anmelden(const std::string& rechnername) {
    openhab::json liste = {{{"type", "Group"},
                            {"name", "gPi"},
                            {"label", "Raspberry Pi " + rechnername},
                            {"tags", {"Equipment"}}}};
    for (const ItemDef& d : ITEMS)
        liste.push_back({{"type", d.typ},
                         {"name", d.name},
                         {"label", d.label},
                         {"groupNames", {"gPi"}}});
    openhab::Items items(client_);
    openhab::json r = items.addOrUpdateItems(liste);  // ein Aufruf, idempotent
    if (openhab::isError(r)) {
        std::cerr << "anmelden: " << openhab::errorMessage(r) << "\n";
        return false;
    }
    for (const ItemDef& d : ITEMS) {
        if (!d.einheit) continue;
        items.addMetadata(d.name, "unit", {{"value", d.einheit}});
        items.addMetadata(d.name, "stateDescription",
                          {{"value", " "}, {"config", {{"pattern", d.muster}}}});
    }
    // Lebenszeichen: nach drei Intervallen ohne Meldung fällt Pi_Online auf OFF
    items.addMetadata("Pi_Online", "expire",
                      {{"value", std::to_string(3 * intervall_) + "s,state=OFF"}});
    // Commands an Lüfter und Aktion bestätigt das Gerät selbst
    items.addMetadata("Pi_Luefter", "autoupdate", {{"value", "false"}});
    items.addMetadata("Pi_Aktion", "autoupdate", {{"value", "false"}});
    items.addMetadata("Pi_Aktion", "stateDescription",
                      {{"value", " "},
                       {"config",
                        {{"options",
                          "ERREICHBARKEIT=openHAB anpingen,"
                          "DATENTRAEGER=Datenträger,ZEIT=Uhrzeit"}}}});
    melde("Pi_Luefter", luefter_.an() ? "ON" : "OFF");
    return true;
}

void Geraet::melden(const Messung& m) {
    // kein Sensor (z. B. in einer VM): UNDEF statt einer erfundenen Zahl
    melde("Pi_Temperatur", m.temperatur ? mitEinheit(*m.temperatur, 1, "°C") : "UNDEF");
    if (m.last) melde("Pi_Last", mitEinheit(*m.last, 0, "%"));
    if (m.speicher) melde("Pi_Speicher", mitEinheit(*m.speicher, 0, "%"));
    if (m.netzKbit) melde("Pi_Netz", mitEinheit(*m.netzKbit, 1, "kbit/s"));
    if (m.laufzeitH) melde("Pi_Laufzeit", mitEinheit(*m.laufzeitH, 1, "h"));
    melde("Pi_Online", "ON");  // Lebenszeichen
}

// Aktionen: nur diese drei, ohne Shell und ohne Argumente von außen
std::string Geraet::aktion(const std::string& name) {
    if (name == "ERREICHBARKEIT") {
        auto start = std::chrono::steady_clock::now();
        client_.get("/");
        std::chrono::duration<double, std::milli> dauer =
            std::chrono::steady_clock::now() - start;
        return "openHAB antwortet in " + mitEinheit(dauer.count(), 0, "ms");
    }
    if (name == "DATENTRAEGER") {
        struct statvfs s;
        if (statvfs("/", &s) != 0) return "Datenträger nicht lesbar";
        double belegt = 100.0 * (s.f_blocks - s.f_bavail) / s.f_blocks;
        double frei = static_cast<double>(s.f_bavail) * s.f_frsize / (1 << 30);
        return "/ zu " + mitEinheit(belegt, 0, "%") + " belegt, " +
               mitEinheit(frei, 1, "GiB") + " frei";
    }
    if (name == "ZEIT") {
        char text[32];
        std::time_t t = std::time(nullptr);
        std::strftime(text, sizeof text, "Gerätezeit %H:%M:%S", std::localtime(&t));
        return text;
    }
    return "unbekannte Aktion: " + name;
}

std::optional<std::string> Geraet::command(const std::string& item,
                                           const std::string& befehl) {
    if (item == "Pi_Luefter") {
        bool an = befehl == "ON";
        if (!luefter_.setzen(an)) return "Lüfter: GPIO-Fehler";
        melde("Pi_Luefter", an ? "ON" : "OFF");  // erst jetzt: bestätigen
        return std::string("Lüfter ") + (an ? "an" : "aus") + " (" + luefter_.name() + ")";
    }
    if (item == "Pi_Aktion") {
        std::string antwort = aktion(befehl);
        melde("Pi_Aktion", befehl);
        melde("Pi_Antwort", antwort);
        return antwort;
    }
    return std::nullopt;
}

void Geraet::abmelden() {
    melde("Pi_Online", "OFF");
}
