// Kapitel 17: Der Controller – verbindet Model und openHAB.
// Commands gehen per REST hinaus, States kommen nur über den Ereignisstrom zurück:
// openHAB bleibt die einzige Quelle der Wahrheit.
#pragma once

#include <openhab/openhab.hpp>

#include <QObject>
#include <QString>
#include <thread>

#include "model.hpp"

class Controller : public QObject {
    Q_OBJECT
public:
    Controller(openhab::OpenHABClient& client, Model& model);
    ~Controller() override { stoppen(); }

    int laden(const QString& gruppe);  // Anzahl Items oder -1
    bool starten();                    // Ereignisstrom in einem eigenen Thread lesen
    void stoppen();
    void umschalten(int index);            // Switch, Dimmer, Rollladen
    void aendern(int index, int schritt);  // Dimmer/Rollladen +/- Prozent
    QString letzterFehler() const { return fehler_; }

signals:
    void fehler(const QString& text);  // jede Fehlermeldung, auch für die Views

private:
    void uebernehmen(const openhab::json& mitglieder);
    void senden(const Eintrag& e, const QString& befehl);
    void melden(const QString& text);

    openhab::OpenHABClient& client_;
    Model& model_;
    openhab::EventStream strom_;
    std::thread leser_;
    QString fehler_;
};
