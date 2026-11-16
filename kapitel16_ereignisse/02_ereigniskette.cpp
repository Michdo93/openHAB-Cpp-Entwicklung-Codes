// Kapitel 16: Welche Ereignisse löst ein einziger Command aus?
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    const std::string ziel = "Wohnzimmer_Licht";

    // Alle Ereignisse dieses Items: Topic openhab/items/Wohnzimmer_Licht/*
    openhab::EventStream strom =
        openhab::Events(client).getEvents("openhab/items/" + ziel + "/*");
    std::thread schalten([&] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        openhab::Items items(client);
        std::string neu = items.getItemState(ziel) == "ON" ? "OFF" : "ON";
        std::cout << ">>> sende Command " << neu << "\n\n" << std::flush;
        items.sendCommand(ziel, neu);
    });
    // Nach 2 s Stille ist die Kette vorbei: nextEvent liefert std::nullopt
    while (std::optional<openhab::json> e = strom.nextEvent(2000)) {
        std::string topic = (*e)["topic"];
        std::cout << std::left << std::setw(25) << openhab::str(*e, "type") << std::setw(14)
                  << topic.substr(topic.rfind('/') + 1) << openhab::str(*e, "payload.value")
                  << "\n";
    }
    schalten.join();
    return 0;
}
