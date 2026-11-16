// Kapitel 15: Systemdienste, Protokollierung, Zugänge und Symbole
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>

using openhab::json;
const std::string LOGGER = "org.openhab.binding.astro";

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Services services(client);
    openhab::Logging logging(client);

    // 1. Systemdienste und ihre Einstellungen (dieselben wie in der Main UI)
    for (const json& s : services.getServices("de"))
        if (s["category"] == "system")
            std::cout << "  " << std::left << std::setw(29) << s["id"].get<std::string>()
                      << " " << s["label"].get<std::string>() << "\n";
    std::cout << "Regionale Einstellungen: "
              << services.getServiceConfig("org.openhab.i18n") << "\n";

    // 2. Log-Level eines Bindings zur Fehlersuche anheben – und zurücksetzen
    auto level = [&] { return openhab::str(logging.getLogger(LOGGER), "loggers.0.level"); };
    std::cout << "\nvorher: " << level() << "\n";
    logging.modifyOrAddLogger(LOGGER, "DEBUG");
    std::cout << "jetzt:  " << level() << "\n";
    logging.removeLogger(LOGGER);  // zurück auf den Standard
    std::cout << "danach: " << level() << "\n";

    // 3. Wer hat Zugang? API-Tokens und Sitzungen des angemeldeten Benutzers
    openhab::Auth auth(client);
    for (const json& t : auth.getAPITokens())
        std::cout << "\nToken \"" << t["name"].get<std::string>() << "\" (Bereich "
                  << t["scope"].get<std::string>() << ") seit "
                  << t["createdTime"].get<std::string>().substr(0, 10) << "\n";
    std::cout << "offene Sitzungen: " << auth.getSessions().size() << "\n";

    // 4. Symbolsätze und Maßeinheiten
    std::cout << "Symbolsätze:";
    for (const json& s : openhab::Iconsets(client).getIconsets("de"))
        std::cout << " " << s["label"].get<std::string>();
    json groessen =
        openhab::at(openhab::Systeminfo(client).getUoMInfo(), "uomInfo.dimensions");
    std::cout << "\n" << groessen.size() << " physikalische Größen, z. B.";
    for (int k = 0; k < 3; k++)
        std::cout << (k ? ", " : " ") << groessen[k]["dimension"].get<std::string>() << " ["
                  << groessen[k]["systemUnit"].get<std::string>() << "]";
    std::cout << "\n";
    return 0;
}
