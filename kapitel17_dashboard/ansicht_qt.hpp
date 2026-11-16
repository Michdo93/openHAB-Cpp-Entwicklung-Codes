// Kapitel 17: Die grafische View – eine Tabelle mit Schaltflächen (Qt Widgets)
#pragma once

#include <QLabel>
#include <QTableView>
#include <QWidget>

#include "controller.hpp"
#include "model.hpp"

class QtAnsicht : public QWidget {
    Q_OBJECT
public:
    QtAnsicht(Model& model, Controller& controller, const QString& titel);

private:
    int gewaehlt() const;  // gewählte Zeile oder -1

    Controller& controller_;
    QTableView tabelle_;
    QLabel status_;
};
