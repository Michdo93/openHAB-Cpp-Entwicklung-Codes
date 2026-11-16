// Kapitel 19: Unit-Tests des Geräts (Kapitel 18) – der Ausgang als GMock-Objekt,
// openHAB als Attrappe
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "attrappe.hpp"
#include "geraet.hpp"

using ::testing::HasSubstr;
using ::testing::Return;

class MockAusgang : public Ausgang {
public:
    MOCK_METHOD(bool, setzen, (bool an), (override));
    MOCK_METHOD(bool, an, (), (const, override));
    MOCK_METHOD(std::string, name, (), (const, override));
};

class GeraetTest : public ::testing::Test {
protected:
    OpenHABAttrappe attrappe;
    openhab::OpenHABClient client{attrappe.url()};
    ::testing::StrictMock<MockAusgang> luefter;  // jeder unerwartete Aufruf ist ein Fehler
    Geraet geraet{client, luefter, 10};
};

TEST_F(GeraetTest, BestaetigtErstNachDemSchalten) {
    EXPECT_CALL(luefter, setzen(true)).WillOnce(Return(true));
    EXPECT_CALL(luefter, name()).WillOnce(Return("gpiochip0:17"));
    auto antwort = geraet.command("Pi_Luefter", "ON");
    ASSERT_TRUE(antwort);
    EXPECT_EQ(*antwort, "Lüfter an (gpiochip0:17)");
    EXPECT_EQ(attrappe.letzte(), "PUT /rest/items/Pi_Luefter/state ON");
}

TEST_F(GeraetTest, GpioFehlerWirdNichtBestaetigt) {
    EXPECT_CALL(luefter, setzen(true)).WillOnce(Return(false));
    size_t vorher = attrappe.anzahl();
    EXPECT_EQ(geraet.command("Pi_Luefter", "ON"), "Lüfter: GPIO-Fehler");
    EXPECT_EQ(attrappe.anzahl(), vorher);  // kein postUpdate: Pi_Luefter bleibt, wie er war
}

TEST_F(GeraetTest, FremdeItemsBleibenUnberuehrt) {
    // StrictMock: setzen() darf hier gar nicht aufgerufen werden
    EXPECT_FALSE(geraet.command("Wohnzimmer_Licht", "ON"));
}

TEST_F(GeraetTest, AktionMeldetAntwort) {
    auto antwort = geraet.command("Pi_Aktion", "ZEIT");
    ASSERT_TRUE(antwort);
    EXPECT_THAT(*antwort, HasSubstr("Gerätezeit"));
    EXPECT_THAT(attrappe.letzte(),
                HasSubstr("PUT /rest/items/Pi_Antwort/state Gerätezeit"));
}

TEST_F(GeraetTest, OhneSensorUndef) {
    Messung m;  // alles std::nullopt
    m.last = 12;
    geraet.melden(m);
    EXPECT_EQ(attrappe.anfrage(1), "PUT /rest/items/Pi_Temperatur/state UNDEF");
    EXPECT_EQ(attrappe.anfrage(2), "PUT /rest/items/Pi_Last/state 12 %");
    EXPECT_EQ(attrappe.letzte(), "PUT /rest/items/Pi_Online/state ON");
}
