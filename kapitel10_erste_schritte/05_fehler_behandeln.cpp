// Kapitel 10: Fehler behandeln
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iostream>
#include <utility>

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    std::string basis = url ? url : "http://127.0.0.1:8080";
    openhab::OpenHABClient client(basis, std::nullopt, std::nullopt, token ? token : "");
    openhab::Items items(client);

    // Alle Methoden melden Ablehnungen als Ergebnis {"error": …} – keine Ausnahme
    std::pair<const char*, const char*> versuche[] = {
        {"Wohnzimer_Licht", "ON"},     // Tippfehler im Namen
        {"Wohnzimmer_Licht", "HALB"},  // ungültiger Command
        {"Flur_Haustuer", "OPEN"},     // Contact: kein Command
        {"Wohnzimmer_Licht", "OFF"},
    };
    for (const auto& [item, command] : versuche) {
        openhab::json r = items.sendCommand(item, command);
        if (openhab::isError(r))
            std::cout << "FEHLER  " << item << " <- " << command << ": "
                      << openhab::errorMessage(r) << "\n";
        else
            std::cout << "OK      " << item << " <- " << command << "\n";
    }

    // Ein Item aus einer .items-Datei lässt sich steuern, aber nicht löschen
    std::cout << "\nlöschen: "
              << openhab::errorMessage(items.deleteItem("Wohnzimmer_Licht")) << "\n";

    // Falsches Token: Der Client merkt es schon beim Verbinden
    openhab::OpenHABClient fremd(basis, std::nullopt, std::nullopt, "oh.falsch.123");
    std::cout << "falsches Token: angemeldet=" << std::boolalpha << fremd.isLoggedIn()
              << " (" << fremd.lastError() << ")\n";
    std::cout << "  Antwort: "
              << openhab::errorMessage(
                     openhab::Items(fremd).getItemState("Wohnzimmer_Licht"))
              << "\n";

    // Netzwerkfehler: ein Server, den es nicht gibt, zwei Sekunden Wartezeit
    openhab::ClientOptions kurz;
    kurz.timeout = 2;
    openhab::OpenHABClient weg("http://127.0.0.1:9", kurz);
    std::cout << "anderer Server: "
              << openhab::errorMessage(openhab::Items(weg).getItemState("Wohnzimmer_Licht"))
              << "\n";
    return 0;
}
