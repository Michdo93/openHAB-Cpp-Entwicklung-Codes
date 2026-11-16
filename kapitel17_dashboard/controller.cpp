// Kapitel 17: Der Controller
#include "controller.hpp"

#include <QMetaObject>

Controller::Controller(openhab::OpenHABClient& client, Model& model)
    : client_(client), model_(model) {}

void Controller::melden(const QString& text) {
    fehler_ = text;
    emit fehler(text);
}

// Alle Items einer Gruppe (rekursiv) in das Model übernehmen, Gruppen selbst auslassen
void Controller::uebernehmen(const openhab::json& mitglieder) {
    for (const openhab::json& i : mitglieder) {
        if (openhab::str(i, "type") == "Group") {
            uebernehmen(i["members"]);
            continue;
        }
        QString name = QString::fromStdString(openhab::str(i, "name"));
        if (model_.suche(name) < 0)
            model_.hinzufuegen({name, QString::fromStdString(openhab::str(i, "label")),
                                QString::fromStdString(openhab::str(i, "type")),
                                QString::fromStdString(openhab::str(i, "state"))});
    }
}

int Controller::laden(const QString& gruppe) {
    openhab::json g = openhab::Items(client_).getItem(gruppe.toStdString(), "");
    if (openhab::isError(g)) {
        melden(QString::fromStdString(openhab::errorMessage(g)));
        return -1;
    }
    uebernehmen(g["members"]);
    return model_.anzahl();
}

bool Controller::starten() {
    strom_ = openhab::ItemEvents(client_).ItemStateChangedEvent();
    if (strom_.isError()) {
        melden(QString::fromStdString(strom_.error()));
        return false;
    }
    leser_ = std::thread([this] {
        for (const openhab::json& e : strom_) {
            std::string topic = e["topic"];
            QString name =
                QString::fromStdString(topic.substr(14, topic.find('/', 14) - 14));
            QString state = QString::fromStdString(openhab::str(e, "payload.value"));
            // Das Model gehört dem Hauptthread: die Änderung dort ausführen lassen
            QMetaObject::invokeMethod(
                &model_, [this, name, state] { model_.setzeState(name, state); },
                Qt::QueuedConnection);
        }
    });
    return true;
}

void Controller::stoppen() {
    strom_.interrupt();  // Leser aufwecken
    if (leser_.joinable()) leser_.join();
}

void Controller::senden(const Eintrag& e, const QString& befehl) {
    openhab::json r = openhab::Items(client_).sendCommand(e.name.toStdString(),
                                                          befehl.toStdString(), "ohdash");
    if (openhab::isError(r)) melden(QString::fromStdString(openhab::errorMessage(r)));
}

void Controller::umschalten(int index) {
    if (index < 0 || index >= model_.anzahl()) return melden("kein solcher Eintrag");
    const Eintrag& e = model_.eintrag(index);
    if (e.typ == "Switch")
        senden(e, e.state == "ON" ? "OFF" : "ON");
    else if (e.typ == "Dimmer" || e.typ == "Color")
        senden(e, e.state.toDouble() > 0 || e.state.contains(',') ? "OFF" : "ON");
    else if (e.typ == "Rollershutter")
        senden(e, e.state.toInt() > 50 ? "UP" : "DOWN");
    else
        melden(e.name + " (" + e.typ + ") lässt sich nicht schalten");
}

void Controller::aendern(int index, int schritt) {
    if (index < 0 || index >= model_.anzahl()) return melden("kein solcher Eintrag");
    const Eintrag& e = model_.eintrag(index);
    if (e.typ != "Dimmer" && e.typ != "Rollershutter")
        return melden(e.name + " ist kein Dimmer");
    int wert = qBound(0, e.state.toInt() + schritt, 100);  // "NULL" ergibt 0
    senden(e, QString::number(wert));
}
