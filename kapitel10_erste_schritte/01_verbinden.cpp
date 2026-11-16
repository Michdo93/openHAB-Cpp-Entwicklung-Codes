// Kapitel 10: Verbindung herstellen und Systeminformationen lesen
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iostream>

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    if (!client.isLoggedIn()) {
        std::cerr << "keine Verbindung: " << client.lastError() << "\n";
        return 1;  // für Skripte und systemd
    }

    openhab::json wurzel = client.get("/");  // /rest/ – Version, Endpunkte
    openhab::json info = openhab::Systeminfo(client).getSystemInfo()["systemInfo"];
    openhab::json uuid = openhab::UUID(client).getUUID();

    std::cout << "openHAB " << openhab::str(wurzel, "runtimeInfo.version") << " unter "
              << client.url() << "\n";
    std::cout << "  Sprache/Land:   " << wurzel["locale"].get<std::string>()
              << ", Maßsystem " << wurzel["measurementSystem"].get<std::string>() << "\n";
    std::cout << "  Endpunkte:      " << wurzel["links"].size() << "\n";
    std::cout << "  Betriebssystem: " << info["osName"].get<std::string>() << " "
              << info["osVersion"].get<std::string>() << " ("
              << info["osArchitecture"].get<std::string>() << ")\n";
    std::cout << "  Java (openHAB): " << info["javaVersion"].get<std::string>() << " von "
              << info["javaVendor"].get<std::string>() << "\n";
    std::cout << "  Konfiguration:  " << info["configFolder"].get<std::string>() << "\n";
    std::cout << "  Speicher:       " << info["freeMemory"].get<long>() / 1048576 << " von "
              << info["totalMemory"].get<long>() / 1048576 << " MiB frei\n";
    std::cout << "  Instanz-UUID:   " << uuid.get<std::string>() << "\n";  // ein String
    std::cout << "  Bibliothek:     cpp-openhab-rest-client " << openhab::VERSION << "\n";
    return 0;
}
