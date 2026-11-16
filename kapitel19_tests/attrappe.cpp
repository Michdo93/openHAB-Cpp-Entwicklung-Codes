// Kapitel 19: Die Attrappe – Sockets, ein Thread, ein Mutex
#include "attrappe.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sstream>
#include <stdexcept>

namespace {
// Eine Anfrage lesen: Kopfzeilen bis zur Leerzeile, dann Content-Length Bytes Body
std::string leseAnfrage(int fd) {
    std::string text;
    char puffer[4096];
    size_t kopfEnde = std::string::npos, laenge = 0;
    for (;;) {
        ssize_t n = recv(fd, puffer, sizeof puffer, 0);
        if (n <= 0) break;
        text.append(puffer, n);
        if (kopfEnde == std::string::npos &&
            (kopfEnde = text.find("\r\n\r\n")) != text.npos) {
            auto cl = text.find("Content-Length:");  // so schreibt es libcurl
            laenge = cl < kopfEnde ? std::stoul(text.substr(cl + 15)) : 0;
        }
        if (kopfEnde != std::string::npos && text.size() - kopfEnde - 4 >= laenge) break;
    }
    return text;
}
}  // namespace

OpenHABAttrappe::OpenHABAttrappe() {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in adr{};
    adr.sin_family = AF_INET;
    adr.sin_port = 0;  // Port 0: frei wählen
    adr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    socklen_t groesse = sizeof adr;
    if (bind(sock_, reinterpret_cast<sockaddr*>(&adr), sizeof adr) || listen(sock_, 16) ||
        getsockname(sock_, reinterpret_cast<sockaddr*>(&adr), &groesse))
        throw std::runtime_error("Attrappe: kein Socket");
    url_ = "http://127.0.0.1:" + std::to_string(ntohs(adr.sin_port));
    thread_ = std::thread([this] { bedienen(); });
}

OpenHABAttrappe::~OpenHABAttrappe() {
    shutdown(sock_, SHUT_RDWR);  // accept() kehrt mit Fehler zurück
    close(sock_);
    thread_.join();
}

void OpenHABAttrappe::bedienen() {
    int fd;
    while ((fd = accept(sock_, nullptr, nullptr)) >= 0) {
        std::string text = leseAnfrage(fd);
        std::istringstream zeile(text);
        std::string methode, pfad;
        zeile >> methode >> pfad;
        auto kopfEnde = text.find("\r\n\r\n");
        std::string body = kopfEnde == text.npos ? "" : text.substr(kopfEnde + 4);

        std::string antwort;
        {
            std::lock_guard<std::mutex> sperre(sperre_);
            anfragen_.push_back(methode + " " + pfad + " " + body);
            Antwort passend{"", "", 200, "{}"};
            for (const Antwort& a : antworten_)
                if (a.methode == methode && pfad.rfind(a.pfad, 0) == 0) passend = a;
            antwort = "HTTP/1.1 " + std::to_string(passend.status) +
                      " X\r\nContent-Type: application/json\r\nContent-Length: " +
                      std::to_string(passend.body.size()) +
                      "\r\nConnection: close\r\n\r\n" + passend.body;
        }
        send(fd, antwort.data(), antwort.size(), 0);
        close(fd);
    }
}

void OpenHABAttrappe::antwort(const std::string& methode, const std::string& pfad,
                              int status, const std::string& body) {
    std::lock_guard<std::mutex> sperre(sperre_);
    antworten_.push_back({methode, pfad, status, body});
}

size_t OpenHABAttrappe::anzahl() const {
    std::lock_guard<std::mutex> sperre(sperre_);
    return anfragen_.size();
}

std::string OpenHABAttrappe::anfrage(size_t i) const {
    std::lock_guard<std::mutex> sperre(sperre_);
    return i < anfragen_.size() ? anfragen_[i] : "";
}

std::string OpenHABAttrappe::letzte() const {
    std::lock_guard<std::mutex> sperre(sperre_);
    return anfragen_.empty() ? "" : anfragen_.back();
}
