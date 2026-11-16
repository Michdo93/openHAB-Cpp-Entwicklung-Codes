// Kapitel 11: Das semantische Modell – Orte, Ausstattung und Messpunkte
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>

using openhab::json;

int main() {
    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    openhab::Items items(client);
    openhab::Tags tags(client);

    // 1. Der Tag-Katalog: Hunderte vordefinierte semantische Tags, hier auf Deutsch
    json katalog = tags.getTags("de");
    int orte = 0;
    std::cout << "Orte, z. B.:";
    for (const json& t : katalog)
        if (openhab::str(t, "uid").rfind("Location_", 0) == 0 && orte++ < 6)
            std::cout << " " << t["label"].get<std::string>() << ",";
    std::cout << " …\n" << katalog.size() << " Tags, davon " << orte << " Orte\n";

    // 2. openHAB leitet aus den Tags Metadaten im Namensraum "semantics" ab
    openhab::Items::GetItemsArgs a;
    a.metadata = "semantics";
    a.fields = "name,label,metadata";
    for (const json& it : items.getItems(a)) {
        json sem = openhab::at(it, "metadata.semantics");
        if (sem.is_null()) continue;
        std::cout << "  " << std::left << std::setw(22) << it["name"].get<std::string>()
                  << " " << std::setw(32) << sem["value"].get<std::string>() << " "
                  << std::setw(21) << openhab::str(sem, "config.relatesTo") << " "
                  << openhab::str(sem, "config.hasLocation") << "\n";
    }

    // 3. Wo befindet sich ein Messpunkt? openHAB folgt dem Modell nach oben.
    json ort = items.getSemanticItem("Wohnzimmer_Temperatur", "Location");
    std::cout << "\nWohnzimmer_Temperatur liegt in: " << openhab::str(ort, "name") << " ("
              << openhab::str(ort, "label") << ")\n";

    // 4. Ein eigener Ort: das Tag wird per REST angelegt und sofort nutzbar
    json tag = tags.createTag({{"uid", "Location_Outdoor_Garden_Gartenhaus"},
                               {"name", "Gartenhaus"},
                               {"label", "Gartenhaus"},
                               {"description", "Hütte im Garten"},
                               {"synonyms", {"Schuppen"}}});
    std::cout << "\nTag angelegt: " << openhab::str(tag, "uid")
              << " (editierbar: " << std::boolalpha << tag.value("editable", false)
              << ")\n";
    items.addOrUpdateItem("gGartenhaus", {{"type", "Group"},
                                          {"name", "gGartenhaus"},
                                          {"label", "Gartenhaus"},
                                          {"tags", {"Gartenhaus"}},
                                          {"groupNames", {"gHaus"}}});
    items.addOrUpdateItem("Gartenhaus_Temperatur",
                          {{"type", "Number:Temperature"},
                           {"name", "Gartenhaus_Temperatur"},
                           {"label", "Temperatur"},
                           {"tags", {"Measurement", "Temperature"}},
                           {"groupNames", {"gGartenhaus"}}});
    json neu = items.getItem("Gartenhaus_Temperatur", "semantics");
    std::cout << "Semantik: " << openhab::str(neu, "metadata.semantics.value") << " "
              << openhab::at(neu, "metadata.semantics.config").dump() << "\n";

    items.deleteItem("Gartenhaus_Temperatur");
    items.deleteItem("gGartenhaus");
    std::cout << "Tag löschen: " << tags.deleteTag("Location_Outdoor_Garden_Gartenhaus")
              << "\n";
    return 0;
}
