// Kapitel 14: Gespeicherte Werte gezielt löschen
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

const std::string ITEM = "Werkstatt_Temperatur";

std::string iso(std::time_t t) {
    std::ostringstream s;
    s << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return s.str();
}

std::string anzahl(openhab::Persistence& persistence, std::time_t jetzt) {
    openhab::Persistence::GetItemPersistenceDataArgs a;
    a.serviceID = "inmemory";
    a.startTime = iso(jetzt - 3 * 86400);
    a.endTime = iso(jetzt);
    return openhab::str(persistence.getItemPersistenceData(ITEM, a), "datapoints", "-1");
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Persistence persistence(client);
    std::time_t jetzt = std::time(nullptr);
    std::cout << "vorher:  " << anzahl(persistence, jetzt) << "\n";

    // Nur die Nacht von 0 bis 6 Uhr von gestern löschen (Ortszeit)
    std::time_t gestern = jetzt - 86400;
    std::tm t = *std::localtime(&gestern);
    t.tm_hour = t.tm_min = t.tm_sec = 0;
    t.tm_isdst = -1;
    std::time_t mitternacht = std::mktime(&t);
    std::string von = iso(mitternacht), bis = iso(mitternacht + 6 * 3600);
    std::cout << persistence.deleteItemData(ITEM, von, bis, "inmemory") << "\n";
    std::cout << "nachher: " << anzahl(persistence, jetzt) << "\n";

    // rrd4j lehnt das Löschen ab – genau wie das Schreiben
    std::string fehler =
        openhab::errorMessage(persistence.deleteItemData(ITEM, von, iso(jetzt), "rrd4j"));
    std::cout << "rrd4j: " << fehler.substr(fehler.rfind('(')) << "\n";
    return 0;
}
