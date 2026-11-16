// Kapitel 18: Ein Schaltausgang – eine Schnittstelle, mehrere Umsetzungen.
//   "gpiochip0:17"  Leitung 17 des Chips gpiochip0 (libgpiod 2, nur mit MIT_GPIOD)
//   "attrappe"      merkt sich den Zustand nur (zum Entwickeln ohne Pi)
// Wer einen Ausgang benutzt, kennt nur die Schnittstelle – Kapitel 19 setzt dort ein
// GMock-Objekt ein.
#pragma once

#include <memory>
#include <string>

class Ausgang {
public:
    virtual ~Ausgang() = default;
    virtual bool setzen(bool an) = 0;  // true = geschaltet
    virtual bool an() const = 0;
    virtual std::string name() const = 0;
};

class AttrappenAusgang : public Ausgang {
public:
    bool setzen(bool an) override {
        an_ = an;
        return true;
    }
    bool an() const override { return an_; }
    std::string name() const override { return "attrappe"; }

private:
    bool an_ = false;
};

// Fabrikfunktion: der passende Ausgang zur Beschreibung, nullptr bei Fehler
std::unique_ptr<Ausgang> ausgangOeffnen(const std::string& beschreibung);
