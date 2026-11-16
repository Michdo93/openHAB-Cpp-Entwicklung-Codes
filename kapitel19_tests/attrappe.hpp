// Kapitel 19: Eine Attrappe für openHAB – ein winziger HTTP-Server im Testprogramm.
// Er beantwortet jede Anfrage mit einer vorbereiteten Antwort und merkt sich, was der
// Client geschickt hat. So lässt sich Code testen, der openHAB aufruft.
#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <vector>

class OpenHABAttrappe {
public:
    OpenHABAttrappe();   // lauscht auf 127.0.0.1:<frei>
    ~OpenHABAttrappe();  // RAII: Socket schließen, Thread beenden
    OpenHABAttrappe(const OpenHABAttrappe&) = delete;
    OpenHABAttrappe& operator=(const OpenHABAttrappe&) = delete;

    const std::string& url() const { return url_; }  // "http://127.0.0.1:<port>"
    // Antwort für alle Pfade, die mit pfad beginnen (zuletzt festgelegte gewinnt)
    void antwort(const std::string& methode, const std::string& pfad, int status,
                 const std::string& body);
    size_t anzahl() const;  // Anzahl bisheriger Anfragen
    // Die i-te Anfrage als "METHODE PFAD BODY"; leer, wenn es sie nicht gibt
    std::string anfrage(size_t i) const;
    std::string letzte() const;

private:
    struct Antwort {
        std::string methode, pfad;
        int status;
        std::string body;
    };
    void bedienen();

    int sock_ = -1;
    std::string url_;
    mutable std::mutex sperre_;
    std::vector<Antwort> antworten_;
    std::vector<std::string> anfragen_;
    std::thread thread_;
};
