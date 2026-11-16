// Kapitel 17: Die Text-View
#include "ansicht_text.hpp"

#include <unistd.h>

#include <QCoreApplication>
#include <QTimer>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

static std::string links(const QString& s, int breite) {  // Zeichen statt Bytes zählen
    return (s + QString(qMax(0, breite - static_cast<int>(s.size())), ' ')).toStdString();
}

TextAnsicht::TextAnsicht(Model& model, Controller& controller, int dauerSekunden)
    : model_(model),
      controller_(controller),
      eingabe_(STDIN_FILENO, QSocketNotifier::Read) {
    // Beobachter des Models: ein Lambda als Slot
    connect(&model_, &Model::stateGeaendert, this, [this](int i) {
        std::cout << "  [" << i + 1 << "] " << links(model_.eintrag(i).name, 22) << " -> "
                  << model_.eintrag(i).state.toStdString() << std::endl;
    });
    connect(&controller_, &Controller::fehler, this,
            [](const QString& t) { std::cout << "  Fehler: " << t.toStdString() << "\n"; });
    connect(&eingabe_, &QSocketNotifier::activated, this, &TextAnsicht::zeileLesen);
    QTimer::singleShot(dauerSekunden * 1000, qApp, &QCoreApplication::quit);
    tabelle();
}

void TextAnsicht::tabelle() const {
    for (int i = 0; i < model_.anzahl(); i++) {
        const Eintrag& e = model_.eintrag(i);
        std::cout << "  [" << i + 1 << "] " << links(e.name, 22) << " " << links(e.typ, 19)
                  << " " << e.state.toStdString() << "\n";
    }
    std::cout << std::flush;
}

void TextAnsicht::zeileLesen() {
    std::string zeile;
    if (!std::getline(std::cin, zeile)) {  // Eingabe zu Ende: nur noch Ereignisse zeigen
        eingabe_.setEnabled(false);
        return;
    }
    std::istringstream ein(zeile);
    char befehl = 0;
    int nr = 0;
    if (!(ein >> befehl)) return;
    ein >> nr;
    if (befehl == 'q') return QCoreApplication::quit();
    std::cout << "> " << zeile << std::endl;
    if (befehl == 't')
        controller_.umschalten(nr - 1);
    else if (befehl == '+')
        controller_.aendern(nr - 1, 10);
    else if (befehl == '-')
        controller_.aendern(nr - 1, -10);
}
