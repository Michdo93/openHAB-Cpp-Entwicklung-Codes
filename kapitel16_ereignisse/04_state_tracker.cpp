// Kapitel 16: Der State-Tracker – nur ausgewählte Items, fertig formatiert
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

// std::setw zählt Bytes, nicht Zeichen – "°" sind zwei Bytes
std::string links(const std::string& s, size_t breite) {
    size_t zeichen = 0;
    for (unsigned char b : s)
        if ((b & 0xC0) != 0x80) zeichen++;
    return s + std::string(zeichen < breite ? breite - zeichen : 0, ' ');
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Events events(client);
    openhab::Items items(client);
    openhab::EventStream strom = events.initiateStateTracker();

    std::string verbindung;
    auto beobachten = [&](openhab::json liste) {  // die Item-Liste der Verbindung
        events.updateSSEConnectionItems(verbindung, liste);
    };
    std::thread bedienen;
    while (std::optional<std::string> zeile = strom.nextLine()) {
        if (verbindung.empty() && zeile->rfind("data:", 0) == 0) {
            // 1. Nachricht: die Verbindungskennung als reiner Text
            verbindung = zeile->substr((*zeile)[5] == ' ' ? 6 : 5);
            std::cout << "Verbindung: " << verbindung << "\n";
            beobachten({"Wohnzimmer_Dimmer", "Wohnzimmer_Temperatur"});
            bedienen = std::thread([&] {
                std::this_thread::sleep_for(1s);
                items.sendCommand("Wohnzimmer_Dimmer", "55");
                items.postUpdate("Werkstatt_Temperatur", "19.3 °C");  // nicht abonniert
                std::this_thread::sleep_for(1s);
                std::cout << "--- Ansicht gewechselt: neue Item-Liste\n" << std::flush;
                beobachten({"Werkstatt_Temperatur", "Werkstatt_Feuchte"});
                std::this_thread::sleep_for(1s);
                items.postUpdate("Werkstatt_Temperatur", "19.8 °C");
                std::this_thread::sleep_for(1s);
                strom.interrupt();
            });
            continue;
        }
        std::optional<openhab::json> daten = openhab::EventStream::parseData(*zeile);
        if (!daten || daten->contains("type")) continue;  // ALIVE überspringen
        for (const auto& [item, z] : daten->items()) {    // {Item: {state, displayState}}
            std::string state = openhab::str(z, "state");
            std::cout << "  " << links(item, 22) << " state=" << links(state, 12)
                      << " anzeige=" << openhab::str(z, "displayState", state) << "\n";
        }
    }
    if (bedienen.joinable()) bedienen.join();
    return 0;
}
