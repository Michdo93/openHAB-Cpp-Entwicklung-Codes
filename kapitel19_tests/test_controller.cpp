// Kapitel 19: Unit-Tests des Controllers (Kapitel 17) gegen die Attrappe statt openHAB
#include <gtest/gtest.h>

#include <QSignalSpy>

#include "attrappe.hpp"
#include "controller.hpp"
#include "model.hpp"

const char* GRUPPE = R"({"name":"gTest","type":"Group","members":[
  {"name":"Licht","type":"Switch","label":"Licht","state":"ON"},
  {"name":"Lampe","type":"Dimmer","label":"Lampe","state":"95"},
  {"name":"gUnter","type":"Group","members":[
    {"name":"Rollo","type":"Rollershutter","label":"Rollo","state":"0"},
    {"name":"Temp","type":"Number","label":"Temp","state":"21"}]}]})";

class ControllerTest : public ::testing::Test {
protected:
    ControllerTest() {
        attrappe.antwort("GET", "/rest/items/gTest", 200, GRUPPE);
        attrappe.antwort("GET", "/rest/items/gFehlt", 404,
                         R"({"error":{"message":"Item gFehlt does not exist!"}})");
        attrappe.antwort("POST", "/rest/items/", 200, "");
    }
    OpenHABAttrappe attrappe;  // Reihenfolge zählt: erst der Server, dann der Client
    openhab::OpenHABClient client{attrappe.url()};
    Model model;
    Controller controller{client, model};
};

TEST_F(ControllerTest, LaedtUntergruppenMit) {
    ASSERT_TRUE(client.isLoggedIn());  // GET /rest/ -> 200 "{}"
    EXPECT_EQ(controller.laden("gTest"), 4);
    EXPECT_EQ(model.eintrag(2).name, "Rollo");
}

TEST_F(ControllerTest, FehlendeGruppe) {
    QSignalSpy spion(&controller, &Controller::fehler);
    EXPECT_EQ(controller.laden("gFehlt"), -1);
    EXPECT_EQ(spion.count(), 1);
    EXPECT_FALSE(controller.letzterFehler().isEmpty());
}

TEST_F(ControllerTest, SendetDiePassendenCommands) {
    controller.laden("gTest");
    controller.umschalten(0);  // Switch ON -> OFF
    EXPECT_EQ(attrappe.letzte(), "POST /rest/items/Licht?source=ohdash OFF");
    controller.aendern(1, 10);  // 95 + 10 -> höchstens 100
    EXPECT_EQ(attrappe.letzte(), "POST /rest/items/Lampe?source=ohdash 100");
    controller.umschalten(2);  // Rollladen 0 -> DOWN
    EXPECT_EQ(attrappe.letzte(), "POST /rest/items/Rollo?source=ohdash DOWN");
}

TEST_F(ControllerTest, NumberLaesstSichNichtSchalten) {
    controller.laden("gTest");
    size_t vorher = attrappe.anzahl();
    controller.umschalten(3);
    EXPECT_EQ(attrappe.anzahl(), vorher);  // nichts gesendet
    EXPECT_TRUE(controller.letzterFehler().contains("nicht schalten"));
}

TEST_F(ControllerTest, ModelAendertSichNurDurchEreignisse) {
    controller.laden("gTest");
    controller.umschalten(0);
    EXPECT_EQ(model.eintrag(0).state, "ON");  // openHAB ist die Quelle der Wahrheit
}
