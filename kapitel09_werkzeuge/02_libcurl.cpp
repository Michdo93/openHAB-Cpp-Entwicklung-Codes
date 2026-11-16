// Kapitel 9: HTTP mit libcurl – was die Bibliothek unter der Haube tut
#include <curl/curl.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

// RAII für die C-Handles von libcurl: unique_ptr mit eigenem "Löscher"
using Curl = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
using Kopf = std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>;

// libcurl liefert die Antwort stückweise; die Rückruffunktion hängt sie an
static size_t sammeln(char* stueck, size_t groesse, size_t anzahl, void* ziel) {
    static_cast<std::string*>(ziel)->append(stueck, groesse * anzahl);
    return groesse * anzahl;
}

static long anfrage(const std::string& methode, const std::string& url,
                    const std::string& token, const std::string* body,
                    std::string& antwort) {
    Curl curl(curl_easy_init(), curl_easy_cleanup);
    curl_slist* l = curl_slist_append(nullptr, ("Authorization: Bearer " + token).c_str());
    l = curl_slist_append(l, "Content-Type: text/plain");
    Kopf kopf(l, curl_slist_free_all);
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, methode.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, kopf.get());
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, sammeln);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &antwort);
    if (body) curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, body->c_str());
    long status = 0;
    CURLcode rc = curl_easy_perform(curl.get());
    if (rc == CURLE_OK)
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);
    else
        std::cerr << "libcurl: " << curl_easy_strerror(rc) << "\n";
    return status;  // Kopf und Curl geben die Destruktoren frei
}

int main() {
    const char* u = std::getenv("OPENHAB_URL");
    const char* t = std::getenv("OPENHAB_TOKEN");
    std::string basis = u ? u : "http://127.0.0.1:8080", token = t ? t : "";
    curl_global_init(CURL_GLOBAL_DEFAULT);  // einmal je Programm

    std::string pfad = "/rest/items/Wohnzimmer_Licht", body = "ON", antwort;
    long status = anfrage("POST", basis + pfad, token, &body, antwort);  // Command
    std::cout << "POST " << pfad << " -> HTTP " << status << "\n";

    antwort.clear();
    status = anfrage("GET", basis + pfad + "/state", token, nullptr, antwort);
    std::cout << "GET  " << pfad << "/state -> HTTP " << status << ": " << antwort << "\n";
    curl_global_cleanup();
    return status == 200 ? 0 : 1;
}
