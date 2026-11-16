// Kapitel 12: Thing-Actions – das Astro-Binding rechnet die Sonnenhöhe aus
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

using openhab::json;
const std::string SONNE = "astro:sun:zuhause";

std::string isoZeit(std::time_t zeit) {
    std::ostringstream s;
    s << std::put_time(std::gmtime(&zeit), "%Y-%m-%dT%H:%M:%SZ");
    return s.str();
}

double hoehe(openhab::Actions& actions, const std::string& uid, std::time_t zeit) {
    json antwort = actions.executeAction(SONNE, uid, {{"date", isoZeit(zeit)}});
    return openhab::num(antwort, "result");  // "-56.97 °" -> -56.97
}

void zeile(openhab::Actions& actions, const std::string& uid, std::time_t zeit) {
    std::cout << "  " << std::put_time(std::localtime(&zeit), "%d.%m., %H:%M") << "  "
              << std::fixed << std::setprecision(1) << std::setw(6)
              << hoehe(actions, uid, zeit) << "°\n";
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Actions actions(client);

    std::string hoeheUid;  // die Action-UID enthält einen Hash hinter '#'
    for (const json& a : actions.getActions(SONNE, "de")) {
        std::string uid = a["actionUid"];
        std::string name = uid.substr(0, uid.find('#'));  // Name = Teil vor dem '#'
        if (name == "astro.getElevation") hoeheUid = uid;
        std::cout << "  " << std::left << std::setw(26) << name << " (";
        bool erste = true;
        for (const json& e : a["inputs"]) {
            std::cout << (erste ? "" : ", ") << e["name"].get<std::string>();
            erste = false;
        }
        std::cout << ") – " << a["description"].get<std::string>() << "\n";
    }
    std::cout << std::right;

    std::cout << "\nSonnenhöhe über den Tag (jetzt und die nächsten Stunden):\n";
    std::time_t jetzt = std::time(nullptr) / 3600 * 3600;  // auf die volle Stunde
    for (int stunden = 0; stunden < 24; stunden += 3)
        zeile(actions, hoeheUid, jetzt + stunden * 3600);
    std::tm t = *std::localtime(&jetzt);
    for (int monat = 3; monat <= 12; monat += 3) {
        t.tm_mon = monat - 1;
        t.tm_mday = 21;
        t.tm_hour = 12;
        t.tm_isdst = -1;  // Sommerzeit selbst bestimmen
        zeile(actions, hoeheUid, std::mktime(&t));
    }
    return 0;
}
