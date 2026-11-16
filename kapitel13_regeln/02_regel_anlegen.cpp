// Kapitel 13: Eine Regel anlegen – Trigger, Bedingung, Aktionen
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using openhab::json;

void zustand(openhab::Items& items, const char* was) {
    std::cout << was;
    const char* sep = "";
    for (const char* name : {"Wohnzimmer_Licht", "Wohnzimmer_Dimmer", "Kueche_Rollladen"}) {
        std::cout << sep << name << "=" << items.getItemState(name).get<std::string>();
        sep = ", ";
    }
    std::cout << "\n";
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Rules rules(client);
    openhab::Items items(client);
    const std::string uid = "buch_kinoabend";

    json regel = json::parse(R"({
      "uid": "buch_kinoabend", "name": "Kinoabend", "tags": ["Buch", "Szene"],
      "description": "Licht dimmen und Rollladen schließen, wenn die Haustür zu ist.",
      "triggers": [{"id": "1", "type": "core.ItemCommandTrigger",
        "configuration": {"itemName": "Haus_Status", "command": "Kino"}}],
      "conditions": [{"id": "2", "type": "core.ItemStateCondition",
        "configuration": {"itemName": "Flur_Haustuer", "operator": "=",
                          "state": "CLOSED"}}],
      "actions": [
        {"id": "3", "type": "core.ItemCommandAction",
         "configuration": {"itemName": "Wohnzimmer_Licht", "command": "OFF"}},
        {"id": "4", "type": "core.ItemCommandAction",
         "configuration": {"itemName": "Wohnzimmer_Dimmer", "command": "20"}},
        {"id": "5", "type": "core.ItemCommandAction",
         "configuration": {"itemName": "Kueche_Rollladen", "command": "DOWN"}}]})");
    rules.deleteRule(uid);  // falls vom letzten Lauf vorhanden
    std::cout << "anlegen: " << rules.createRule(regel) << "\n";
    std::cout << "Status:  " << openhab::str(rules.getRule(uid), "status.status") << "\n";

    // Ausgangslage herstellen
    items.sendCommand("Wohnzimmer_Licht", "ON");
    items.sendCommand("Wohnzimmer_Dimmer", "80");
    items.sendCommand("Kueche_Rollladen", "UP");
    // 1. Tür offen: die Bedingung verhindert die Aktionen
    items.postUpdate("Flur_Haustuer", "OPEN");
    items.sendCommand("Haus_Status", "Kino");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    zustand(items, "Tür offen: ");
    // 2. Tür zu: die Regel greift
    items.postUpdate("Flur_Haustuer", "CLOSED");
    items.sendCommand("Haus_Status", "Kino");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    zustand(items, "Tür zu:    ");
    return 0;
}
