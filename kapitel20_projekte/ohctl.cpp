// Kapitel 20: ohctl – openHAB auf der Kommandozeile, für Menschen und für Skripte
//   ohctl [--json] info | items [Muster] | get Item | send Item Command
//         | update Item State | watch [Item…] | things | rules | run Regel
// Exit-Code: 0 = erledigt, 1 = openHAB meldet einen Fehler, 2 = falscher Aufruf
#include <openhab/openhab.hpp>

#include <fnmatch.h>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {
bool alsJson = false;  // --json: Antworten unverändert ausgeben
std::atomic<openhab::EventStream*> strom{nullptr};

extern "C" void stopp(int) {
    if (openhab::EventStream* s = strom.load()) s->interrupt();
}

int nutzung() {
    std::cerr << "Aufruf: ohctl [--json] BEFEHL …\n"
                 "  info                  openHAB-Version und System\n"
                 "  items [Muster]        Items, z. B. 'Wohnzimmer_*'\n"
                 "  get ITEM              Zustand lesen\n"
                 "  send ITEM COMMAND     Command senden\n"
                 "  update ITEM STATE     Zustand melden\n"
                 "  watch [ITEM …]        Änderungen verfolgen (Strg+C beendet)\n"
                 "  things | rules        Things bzw. Regeln mit Status\n"
                 "  run REGEL             Regel sofort ausführen\n"
                 "Umgebung: OPENHAB_URL, OPENHAB_TOKEN\n";
    return 2;
}

// Gemeinsamer Ausgang: Fehler nach stderr und Exit-Code 1, sonst JSON auf Wunsch
int fertig(const openhab::json& r) {
    if (openhab::isError(r)) {
        std::cerr << "ohctl: " << openhab::errorMessage(r) << "\n";
        return 1;
    }
    if (alsJson)
        std::cout << r.dump(2) << "\n";
    else if (r.is_string())
        std::cout << r.get<std::string>() << "\n";
    return 0;
}

// Eine Tabelle aus drei Feldern je Eintrag – für items, things und rules
int tabelle(const openhab::json& r, const std::vector<std::string>& felder,
            const std::vector<int>& breiten, const char* muster = nullptr) {
    if (alsJson || openhab::isError(r)) return fertig(r);
    for (const openhab::json& e : r) {
        if (muster && fnmatch(muster, openhab::str(e, felder[0]).c_str(), 0) != 0) continue;
        for (size_t i = 0; i < felder.size(); i++)
            std::cout << std::left << std::setw(i + 1 < felder.size() ? breiten[i] : 0)
                      << openhab::str(e, felder[i]) << (i + 1 < felder.size() ? " " : "\n");
    }
    return 0;
}

int watch(openhab::OpenHABClient& c, const std::vector<std::string>& namen) {
    std::string topics;
    for (const std::string& n : namen)  // Liste der Topics, durch Komma getrennt
        topics += (topics.empty() ? "" : ",") + ("openhab/items/" + n + "/statechanged");
    openhab::EventStream s = openhab::Events(c).getEvents(
        topics.empty() ? "openhab/items/*/statechanged" : topics);
    if (s.isError()) return fertig(s.errorResult());
    strom = &s;
    std::signal(SIGINT, stopp);
    std::signal(SIGTERM, stopp);
    for (const openhab::json& e : s) {
        if (alsJson) {
            std::cout << e.dump() << std::endl;  // endl: sofort weiterreichen (Pipes!)
            continue;
        }
        std::string topic = e["topic"];
        std::cout << topic.substr(14, topic.find('/', 14) - 14) << " "
                  << openhab::str(e, "payload.oldValue") << " -> "
                  << openhab::str(e, "payload.value") << std::endl;
    }
    strom = nullptr;
    return 0;
}
}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (!args.empty() && args[0] == "--json") {
        alsJson = true;
        args.erase(args.begin());
    }
    if (args.empty()) return nutzung();
    std::string befehl = args[0];
    std::vector<std::string> arg(args.begin() + 1, args.end());

    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient c(url ? url : "http://127.0.0.1:8080",
                             {std::nullopt, std::nullopt, token ? token : "", 5.0});
    openhab::Items items(c);

    // Befehlstabelle: Name → (erlaubte Argumentzahl, Lambda)
    struct Befehl {
        size_t min, max;
        std::function<int()> ausfuehren;
    };
    const std::map<std::string, Befehl> befehle = {
        {"info",
         {0, 0,
          [&] {
              openhab::json r = c.get("/");
              if (alsJson || openhab::isError(r)) return fertig(r);
              std::cout << "openHAB " << openhab::str(r, "runtimeInfo.version") << " ("
                        << openhab::str(r, "runtimeInfo.buildString") << "), "
                        << openhab::str(r, "locale") << ", " << r["links"].size()
                        << " Endpunkte\n";
              return 0;
          }}},
        {"items",
         {0, 1,
          [&] {
              openhab::Items::GetItemsArgs a;
              a.fields = "name,type,state,label";
              return tabelle(items.getItems(a), {"name", "type", "state"}, {28, 22},
                             arg.empty() ? nullptr : arg[0].c_str());
          }}},
        {"get", {1, 1, [&] { return fertig(items.getItemState(arg[0])); }}},
        {"send",
         {2, 2, [&] { return fertig(items.sendCommand(arg[0], arg[1], "ohctl")); }}},
        {"update",
         {2, 2, [&] { return fertig(items.postUpdate(arg[0], arg[1], "ohctl")); }}},
        {"watch", {0, 100, [&] { return watch(c, arg); }}},
        {"things",
         {0, 0,
          [&] {
              openhab::Things::GetThingsArgs a;
              a.summary = true;
              return tabelle(openhab::Things(c).getThings(a),
                             {"UID", "label", "statusInfo.status"}, {24, 28});
          }}},
        {"rules",
         {0, 0,
          [&] {
              openhab::Rules::GetRulesArgs a;
              a.summary = true;
              return tabelle(openhab::Rules(c).getRules(a),
                             {"uid", "name", "status.status"}, {24, 28});
          }}},
        {"run", {1, 1, [&] { return fertig(openhab::Rules(c).runNow(arg[0])); }}},
    };
    auto b = befehle.find(befehl);
    if (b == befehle.end() || arg.size() < b->second.min || arg.size() > b->second.max)
        return nutzung();
    return b->second.ausfuehren();
}
