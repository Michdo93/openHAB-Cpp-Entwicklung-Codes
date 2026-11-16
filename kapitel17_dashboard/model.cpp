// Kapitel 17: Das Model – es lebt im Hauptthread; der Controller ruft setzeState() nur
// dort auf, darum braucht es keinen Mutex
#include "model.hpp"

int Model::hinzufuegen(const Eintrag& e) {
    beginInsertRows({}, anzahl(), anzahl());  // Views über die neue Zeile informieren
    eintraege_.push_back(e);
    endInsertRows();
    return anzahl() - 1;
}

int Model::suche(const QString& name) const {
    for (int i = 0; i < anzahl(); i++)
        if (eintraege_[i].name == name) return i;
    return -1;
}

bool Model::setzeState(const QString& name, const QString& state) {
    int i = suche(name);
    if (i < 0 || eintraege_[i].state == state) return false;
    eintraege_[i].state = state;
    emit dataChanged(index(i, State), index(i, State));  // für Qt-Views
    emit stateGeaendert(i);                              // für alle anderen Beobachter
    return true;
}

QVariant Model::data(const QModelIndex& index, int rolle) const {
    const Eintrag& e = eintraege_.at(index.row());
    if (rolle == Qt::DisplayRole) {
        switch (index.column()) {
        case Label:
            return e.label;
        case Name:
            return e.name;
        case Typ:
            return e.typ;
        case State:
            return e.state;
        }
    }
    if (rolle == Qt::TextAlignmentRole && index.column() == State)
        return int(Qt::AlignRight | Qt::AlignVCenter);  // Werte rechtsbündig
    return {};
}

QVariant Model::headerData(int abschnitt, Qt::Orientation o, int rolle) const {
    if (rolle != Qt::DisplayRole || o != Qt::Horizontal) return {};
    static const char* titel[] = {"Bezeichnung", "Item", "Typ", "State"};
    return QString(titel[abschnitt]);
}
