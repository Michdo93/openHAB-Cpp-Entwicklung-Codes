// Kapitel 13: Eine Regel steuern – Parameter ändern, ausführen, abschalten, ersetzen
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using openhab::json;
using namespace std::chrono_literals;

const std::string UID = "buch_kinoabend";  // aus 02_regel_anlegen.cpp

// Texte ohne, JSON mit Anführungszeichen ausgeben
std::string text(const json& r) {
    return r.is_string() ? r.get<std::string>() : r.dump();
}

void status(openhab::Rules& rules, const char* was) {
    json r = rules.getRule(UID);
    std::cout << was << openhab::str(r, "status.status") << " / "
              << openhab::str(r, "status.statusDetail") << "\n";
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Rules rules(client);
    openhab::Items items(client);
    if (openhab::isError(rules.getRule(UID))) {
        std::cerr << "Zuerst 02_regel_anlegen ausführen.\n";
        return 1;
    }

    // 1. Einen einzelnen Parameter eines Moduls ändern: Dimmer auf 10 statt 20 %
    rules.setModuleConfigParam(UID, "actions", "4", "command", "10");
    std::cout << "Dimmwert jetzt: "
              << text(rules.getModuleConfigParam(UID, "actions", "4", "command")) << "\n";
    // 2. Sofort ausführen – runNow überspringt Trigger UND Bedingungen
    items.sendCommand("Wohnzimmer_Dimmer", "80");
    items.postUpdate("Flur_Haustuer", "OPEN");
    rules.runNow(UID);
    std::this_thread::sleep_for(1s);
    std::cout << "runNow trotz offener Tür: "
              << text(items.getItemState("Wohnzimmer_Dimmer")) << "\n";

    // 3. Abschalten: der Trigger wird ignoriert
    rules.disable(UID);
    status(rules, "abgeschaltet: ");
    items.postUpdate("Flur_Haustuer", "CLOSED");
    items.sendCommand("Wohnzimmer_Dimmer", "80");
    items.sendCommand("Haus_Status", "Kino");
    std::this_thread::sleep_for(1s);
    std::cout << "Trigger bei abgeschalteter Regel: "
              << text(items.getItemState("Wohnzimmer_Dimmer")) << "\n";
    rules.enable(UID);
    status(rules, "eingeschaltet: ");

    // 4. Die ganze Regel ersetzen: eine zusätzliche Aktion anhängen
    json regel = rules.getRule(UID);
    regel["actions"].push_back(
        {{"id", "6"},
         {"type", "core.ItemStateUpdateAction"},
         {"configuration", {{"itemName", "Haus_Status"}, {"state", "Film läuft"}}}});
    rules.updateRule(UID, regel);
    std::this_thread::sleep_for(1s);  // openHAB initialisiert die Regel neu
    items.sendCommand("Haus_Status", "Kino");
    std::this_thread::sleep_for(1s);
    std::cout << "nach updateRule: " << text(items.getItemState("Haus_Status")) << "\n";
    std::cout << "löschen: " << text(rules.deleteRule(UID)) << "\n";
    return 0;
}
