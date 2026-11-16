// Kapitel 19: Unit-Tests der Zerlege-Funktionen (Kapitel 18) – ganz ohne Rechner
#include <gtest/gtest.h>

#include "messwerte.hpp"

using namespace messwerte;

TEST(Messwerte, Temperatur) {
    EXPECT_DOUBLE_EQ(*temperaturAus("48312\n"), 48.312);
    EXPECT_FALSE(temperaturAus(""));  // kein Sensor
    EXPECT_FALSE(temperaturAus("abc"));
}

TEST(Messwerte, LastJeKern) {
    EXPECT_DOUBLE_EQ(*lastAus("0.52 0.40 0.31 1/123 4567\n", 4), 13.0);
    EXPECT_FALSE(lastAus("0.52", 0));  // keine Kerne: kein Ergebnis statt Division durch 0
}

TEST(Messwerte, Speicher) {
    const char* meminfo =
        "MemTotal:        8000000 kB\nMemFree:  1000000 kB\nMemAvailable:    6000000 kB\n";
    EXPECT_DOUBLE_EQ(*speicherAus(meminfo), 25.0);
    EXPECT_FALSE(speicherAus("MemTotal: 8000000 kB\n"));  // MemAvailable fehlt
}

TEST(Messwerte, NetzOhneLoopback) {
    const char* netdev =
        "Inter-|   Receive                            |  Transmit\n"
        " face |bytes    packets errs drop fifo frame compressed multicast|bytes ...\n"
        "    lo: 5000 10 0 0 0 0 0 0 5000 10 0 0 0 0 0 0\n"
        "  eth0: 1000 10 0 0 0 0 0 0 200 3 0 0 0 0 0 0\n"
        " wlan0: 30 1 0 0 0 0 0 0 4 1 0 0 0 0 0 0\n";
    EXPECT_EQ(*netzbytesAus(netdev), 1234);
    EXPECT_FALSE(netzbytesAus(""));
}
