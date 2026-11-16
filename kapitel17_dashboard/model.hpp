// Kapitel 17: Das Model – der Zustand der Items, die das Dashboard zeigt.
// Es kennt weder openHAB noch die Oberfläche. Als QAbstractTableModel passt es direkt
// an jede Qt-View; wer sich für Änderungen interessiert, verbindet sich mit dem Signal
// stateGeaendert – Signale und Slots sind Qts Form des Observer-Musters.
#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QVector>

struct Eintrag {
    QString name, label, typ, state;
};

class Model : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Spalte { Label, Name, Typ, State, AnzahlSpalten };

    using QAbstractTableModel::QAbstractTableModel;

    int hinzufuegen(const Eintrag& e);     // Index des neuen Eintrags
    int suche(const QString& name) const;  // Index oder -1
    const Eintrag& eintrag(int index) const { return eintraege_.at(index); }
    int anzahl() const { return eintraege_.size(); }
    bool setzeState(const QString& name, const QString& state);  // true = geändert

    // Schnittstelle für Qt-Views
    int rowCount(const QModelIndex& = {}) const override { return anzahl(); }
    int columnCount(const QModelIndex& = {}) const override { return AnzahlSpalten; }
    QVariant data(const QModelIndex& index, int rolle) const override;
    QVariant headerData(int abschnitt, Qt::Orientation o, int rolle) const override;

signals:
    void stateGeaendert(int index);  // nach jeder Änderung eines States

private:
    QVector<Eintrag> eintraege_;
};
