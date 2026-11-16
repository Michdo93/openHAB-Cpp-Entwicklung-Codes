// Kapitel 17: Die Text-View – jede Änderung eine Zeile, Befehle zeilenweise über stdin
//   t <Nr>   umschalten      + <Nr>  heller     - <Nr>  dunkler      q  beenden
#pragma once

#include <QObject>
#include <QSocketNotifier>

#include "controller.hpp"
#include "model.hpp"

class TextAnsicht : public QObject {
    Q_OBJECT
public:
    TextAnsicht(Model& model, Controller& controller, int dauerSekunden);
    void tabelle() const;

private:
    void zeileLesen();

    Model& model_;
    Controller& controller_;
    QSocketNotifier eingabe_;
};
