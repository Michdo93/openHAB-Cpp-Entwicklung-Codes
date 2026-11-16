// Kapitel 18: Die Logik des Geräts – Philosophie 2: Das Programm ist die Quelle der
// Wahrheit für seine Items. Es MELDET Werte per postUpdate, EMPFÄNGT Commands über den
// Ereignisstrom und BESTÄTIGT sie, wenn die Hardware geschaltet hat.
#pragma once

#include <openhab/openhab.hpp>

#include <optional>
#include <string>

#include "ausgang.hpp"
#include "messwerte.hpp"

class Geraet {
public:
    Geraet(openhab::OpenHABClient& client, Ausgang& luefter, int intervall)
        : client_(client), luefter_(luefter), intervall_(intervall) {}

    bool anmelden(const std::string& rechnername);  // Items anlegen (idempotent)
    void melden(const Messung& m);                  // Werte und Lebenszeichen
    // Einen Command ausführen; liefert die Antwort für Pi_Antwort, nullopt = fremdes Item
    std::optional<std::string> command(const std::string& item, const std::string& befehl);
    void abmelden();  // Lebenszeichen OFF

private:
    void melde(const std::string& item, const std::string& state);
    std::string aktion(const std::string& name);

    openhab::OpenHABClient& client_;
    Ausgang& luefter_;
    int intervall_;
};

// "47.5 °C" aus 47.5, 1, "°C" – die Zahlformatierung des Geräts
std::string mitEinheit(double wert, int stellen, const std::string& einheit);
