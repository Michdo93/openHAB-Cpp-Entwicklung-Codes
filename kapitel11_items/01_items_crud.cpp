// Kapitel 11: Ein Item anlegen, lesen, ändern und löschen (CRUD)
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);
    const std::string name = "Garten_Pumpe";

    // Create – die Definition als JSON-Objekt
    json definition = {{"type", "Switch"},       {"name", name},
                       {"label", "Gartenpumpe"}, {"category", "water"},
                       {"tags", {"Switch"}},     {"groupNames", {"gHaus"}}};
    json neu = items.addOrUpdateItem(name, definition);
    std::cout << "angelegt:  " << neu["name"].get<std::string>() << " "
              << neu["type"].get<std::string>() << " " << neu["state"].get<std::string>()
              << "\n";

    // Read – und gleich bedienen: ein per REST angelegtes Item ist ein normales Item
    items.sendCommand(name, "ON");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));  // wirkt asynchron
    std::cout << "gelesen:   " << items.getItemState(name).get<std::string>() << "\n";

    // Update – PUT ersetzt die ganze Definition (idempotent)
    definition["label"] = "Gartenpumpe Zisterne";
    definition["groupNames"].push_back("gWerkstatt");
    items.addOrUpdateItem(name, definition);

    json pumpe = items.getItem(name);
    std::cout << "geändert:  " << pumpe["label"].get<std::string>() << " "
              << pumpe["groupNames"].dump()
              << " Zustand: " << pumpe["state"].get<std::string>() << "\n";
    std::cout << "verwaltet: "
              << (pumpe.value("editable", false) ? "per REST änderbar" : "aus einer Datei")
              << "\n";

    // Delete – und prüfen, dass es weg ist
    std::cout << "löschen:   " << items.deleteItem(name).dump() << "\n";
    std::cout << "danach:    " << openhab::errorMessage(items.getItem(name)) << "\n";
    return 0;
}
