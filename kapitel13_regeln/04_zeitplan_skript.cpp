// Kapitel 13: Zeitplan-Trigger, ein Rules-DSL-Skript und die Vorschau des Zeitplans
#include <openhab/openhab.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

using openhab::json;

const std::string UID = "buch_lueften";

// Rules-DSL-Skript als Aktion: läuft werktags um 7:30 und 17:30
const char* SKRIPT = R"(val feuchte = Werkstatt_Feuchte.state
if (feuchte instanceof QuantityType
        && (feuchte as QuantityType<?>).toUnit("%").doubleValue > 60) {
    Werkstatt_Luefter.sendCommand(ON)
    Haus_Status.postUpdate("Werkstatt wird gelüftet")
}
)";

json cron(const std::string& id, const std::string& ausdruck) {
    return {{"id", id},
            {"type", "timer.GenericCronTrigger"},
            {"configuration", {{"cronExpression", ausdruck}}}};
}

std::string utc(std::time_t t) {
    std::ostringstream s;
    s << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return s.str();
}

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Rules rules(client);
    openhab::Items items(client);

    json regel = {
        {"uid", UID},
        {"name", "Werkstatt lüften"},
        {"tags", {"Buch", "Schedule"}},
        {"triggers",
         {cron("morgens", "0 30 7 ? * MON-FRI"), cron("abends", "0 30 17 ? * MON-FRI")}},
        {"actions",
         {{{"id", "skript"},
           {"type", "script.ScriptAction"},
           {"configuration",
            {{"type", "application/vnd.openhab.dsl.rule"}, {"script", SKRIPT}}}}}}};
    rules.deleteRule(UID);
    std::cout << rules.createRule(regel) << "\n";

    // Wann läuft was? openHAB simuliert alle Regeln mit dem Tag "Schedule"
    std::time_t morgen = std::time(nullptr) + 86400;
    std::tm t = *std::localtime(&morgen);
    t.tm_hour = t.tm_min = t.tm_sec = 0;
    std::time_t start = std::mktime(&t);
    for (const json& e : rules.simulateSchedule(utc(start), utc(start + 7 * 86400))) {
        if (openhab::str(e, "rule.uid") != UID) continue;
        // openHAB liefert z. B. "Nov 16, 2026, 7:30:00 AM" (mit schmalem Leerzeichen)
        std::string datum;
        for (unsigned char z : e["date"].get<std::string>())
            if (z < 0x80)
                datum += static_cast<char>(z);  // nur ASCII behalten
            else if (z == 0xE2)
                datum += ' ';  // U+202F -> Leerzeichen
        std::tm w{};
        std::istringstream(datum) >> std::get_time(&w, "%b %d, %Y, %I:%M:%S %p");
        w.tm_isdst = -1;
        std::mktime(&w);  // ergänzt den Wochentag (tm_wday)
        static const char* TAG[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
        std::cout << "  " << TAG[w.tm_wday] << " " << std::put_time(&w, "%d.%m., %H:%M")
                  << "  " << openhab::str(e, "rule.name") << "\n";
    }

    // Nicht bis morgen warten: Werkstatt feucht machen und die Regel sofort ausführen
    items.postUpdate("Werkstatt_Feuchte", "72 %");
    rules.runNow(UID);
    std::this_thread::sleep_for(std::chrono::seconds(2));  // DSL wird erst übersetzt
    std::cout << "Status: " << items.getItemState("Haus_Status").get<std::string>() << "\n";
    std::cout << "Lüfter: " << items.getItemState("Werkstatt_Luefter").get<std::string>()
              << " (autoupdate=false: erst das Gerät bestätigt den Command)\n";
    std::cout << "löschen: " << rules.deleteRule(UID).dump() << "\n";
    return 0;
}
