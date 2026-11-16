// Kapitel 19: Unit-Tests des Models (Kapitel 17) – Signale prüfen mit QSignalSpy
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "model.hpp"

class ModelTest : public ::testing::Test {  // Fixture: für jeden Test ein frisches Model
protected:
    void SetUp() override {
        model.hinzufuegen({"Licht", "Deckenlicht", "Switch", "OFF"});
        model.hinzufuegen({"Lampe", "Leselampe", "Dimmer", "40"});
    }
    Model model;
};

TEST_F(ModelTest, Suchen) {
    EXPECT_EQ(model.anzahl(), 2);
    EXPECT_EQ(model.suche("Lampe"), 1);
    EXPECT_EQ(model.suche("Fehlt"), -1);
}

TEST_F(ModelTest, AenderungMeldetGenauEinmal) {
    QSignalSpy spion(&model, &Model::stateGeaendert);
    EXPECT_TRUE(model.setzeState("Lampe", "60"));
    EXPECT_FALSE(model.setzeState("Lampe", "60"));  // gleicher Wert: kein Signal
    EXPECT_FALSE(model.setzeState("Fehlt", "ON"));  // unbekanntes Item
    ASSERT_EQ(spion.count(), 1);
    EXPECT_EQ(spion.at(0).at(0).toInt(), 1);  // Argument: der Index
    EXPECT_EQ(model.eintrag(1).state, "60");
}

TEST_F(ModelTest, QtViewSiehtDieDaten) {
    QSignalSpy spion(&model, &QAbstractItemModel::dataChanged);
    model.setzeState("Licht", "ON");
    EXPECT_EQ(spion.count(), 1);
    EXPECT_EQ(model.data(model.index(0, Model::State), Qt::DisplayRole).toString(), "ON");
    EXPECT_EQ(model.rowCount(), 2);
}
