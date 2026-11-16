// Kapitel 17: Die grafische View
#include "ansicht_qt.hpp"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTime>
#include <QVBoxLayout>

QtAnsicht::QtAnsicht(Model& model, Controller& controller, const QString& titel)
    : controller_(controller) {
    setWindowTitle("ohdash – " + titel);
    tabelle_.setModel(&model);  // die View liest das Model selbst
    tabelle_.setSelectionBehavior(QAbstractItemView::SelectRows);
    tabelle_.setSelectionMode(QAbstractItemView::SingleSelection);
    tabelle_.verticalHeader()->hide();
    tabelle_.horizontalHeader()->setStretchLastSection(true);
    tabelle_.horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto* schalten = new QPushButton("Schalten");
    auto* heller = new QPushButton("Heller +10");
    auto* dunkler = new QPushButton("Dunkler −10");
    auto* knoepfe = new QHBoxLayout;
    for (QPushButton* k : {schalten, heller, dunkler}) knoepfe->addWidget(k);
    knoepfe->addStretch();
    auto* layout = new QVBoxLayout(this);  // Qt gibt die Kinder mit dem Fenster frei
    layout->addWidget(&tabelle_);
    layout->addLayout(knoepfe);
    layout->addWidget(&status_);

    // Eingaben gehen an den Controller – nie direkt an das Model
    connect(schalten, &QPushButton::clicked, this,
            [this] { controller_.umschalten(gewaehlt()); });
    connect(heller, &QPushButton::clicked, this,
            [this] { controller_.aendern(gewaehlt(), 10); });
    connect(dunkler, &QPushButton::clicked, this,
            [this] { controller_.aendern(gewaehlt(), -10); });
    connect(&tabelle_, &QTableView::doubleClicked, this,
            [this](const QModelIndex& i) { controller_.umschalten(i.row()); });
    // Rückmeldungen: Änderungen aus dem Model, Fehler aus dem Controller
    connect(&model, &Model::stateGeaendert, this, [this, &model](int i) {
        status_.setText(QTime::currentTime().toString() + "  " + model.eintrag(i).label +
                        ": " + model.eintrag(i).state);
    });
    connect(&controller_, &Controller::fehler, this,
            [this](const QString& t) { status_.setText("Fehler: " + t); });
    resize(640, 260);
}

int QtAnsicht::gewaehlt() const {
    QModelIndexList zeilen = tabelle_.selectionModel()->selectedRows();
    return zeilen.isEmpty() ? -1 : zeilen.first().row();
}
