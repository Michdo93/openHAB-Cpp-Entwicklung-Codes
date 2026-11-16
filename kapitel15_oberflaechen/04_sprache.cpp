// Kapitel 15: Text wie einen Sprachbefehl auswerten lassen – ohne Mikrofon
//   ./starte.sh kapitel15_oberflaechen/04_sprache.cpp "Schalte die Leselampe ein"
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

using openhab::json;

// Der Interpreter erkennt Items an ihrem Label und antwortet in der gewählten Sprache
void sage(openhab::Voice& voice, const std::string& satz) {
    json antwort = voice.interpretText(satz, "de");
    std::string fehler = openhab::errorMessage(antwort);
    std::cout << "  »" << satz << "« -> "
              << (fehler.empty() ? antwort.get<std::string>()
                                 : fehler.substr(fehler.rfind('(')))
              << "\n";
}

int main(int argc, char** argv) {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Voice voice(client);
    openhab::Items items(client);

    for (const json& i : voice.getInterpreters("de"))
        std::cout << "Interpreter " << std::left << std::setw(8)
                  << i["id"].get<std::string>() << " " << std::setw(26)
                  << i["label"].get<std::string>() << " Sprachen "
                  << i["locales"].dump().substr(0, 30) << "…\n";
    std::cout << "Audio-Ausgänge:";
    for (const json& s : openhab::Audio(client).getSinks())
        std::cout << " " << s["id"].get<std::string>();
    std::cout << "\n";

    // Den eingebauten Interpreter zum Standard machen – eine Systemeinstellung
    openhab::Services services(client);
    json config = services.getServiceConfig("org.openhab.voice");
    config["defaultHLI"] = "system";
    services.updateServiceConfig("org.openhab.voice", config);
    std::cout << "Standard-Interpreter: "
              << openhab::str(services.getServiceConfig("org.openhab.voice"), "defaultHLI")
              << "\n";

    items.sendCommand("Wohnzimmer_Licht", "OFF");
    items.sendCommand("Wohnzimmer_Dimmer", "60");
    for (const char* satz : {"Schalte das Deckenlicht ein", "Schalte das Deckenlicht ein",
                             "Schalte die Leselampe aus", "Mach mir einen Kaffee"})
        sage(voice, satz);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Deckenlicht: "
              << items.getItemState("Wohnzimmer_Licht").get<std::string>()
              << " | Leselampe: "
              << items.getItemState("Wohnzimmer_Dimmer").get<std::string>() << "\n";

    // Einen bestimmten Interpreter wählen, unabhängig vom Standard
    json r =
        voice.interpretTextBatch("turn off the Deckenlicht", json::array({"system"}), "en");
    std::cout << (r.is_string() ? r.get<std::string>() : openhab::errorMessage(r)) << "\n";

    for (int k = 1; k < argc; k++) sage(voice, argv[k]);  // eigene Sätze aus dem Terminal
    return 0;
}
