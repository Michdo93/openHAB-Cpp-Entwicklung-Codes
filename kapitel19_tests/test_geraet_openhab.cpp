// Kapitel 19: Integrationstest des Geräts (Kapitel 18) mit openHAB und der Test-Suite
#include <gtest/gtest.h>
#include <openhab/testsuite.hpp>

#include <chrono>
#include <cstdlib>
#include <thread>

#include "geraet.hpp"

class GeraetOpenHAB : public ::testing::Test {
protected:
    void SetUp() override {
        if (!token || !*token) GTEST_SKIP() << "OPENHAB_TOKEN fehlt";
        openhab::testsuite::setLog([](int, const std::string&) {});  // Suite leise
    }
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client{url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : ""};
    AttrappenAusgang luefter;
    Geraet geraet{client, luefter, 10};
    openhab::testsuite::ItemTester items{client};
};

TEST_F(GeraetOpenHAB, AnmeldenIstIdempotent) {
    EXPECT_TRUE(geraet.anmelden("test"));
    EXPECT_TRUE(geraet.anmelden("test"));  // zweimal hintereinander: kein Fehler
    EXPECT_TRUE(items.checkItemIsType("Pi_Temperatur", "Number:Temperature"));
    EXPECT_TRUE(items.doesGroupContainMember("gPi", "Pi_Luefter"));
}

TEST_F(GeraetOpenHAB, MeldenUndBestaetigen) {
    ASSERT_TRUE(geraet.anmelden("test"));
    geraet.melden({47.5, 12, 40, 8, 2.5});
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(items.checkItemHasState("Pi_Temperatur", "47.5 °C"));
    EXPECT_TRUE(items.checkItemHasState("Pi_Online", "ON"));

    EXPECT_TRUE(geraet.command("Pi_Luefter", "ON"));
    EXPECT_TRUE(luefter.an());
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(items.checkItemHasState("Pi_Luefter", "ON"));
    geraet.command("Pi_Luefter", "OFF");
    geraet.abmelden();
}
