// Kapitel 9: Signale – ein Dienst beendet sich sauber bei Strg+C oder systemctl stop
#include <csignal>
#include <iostream>

#include <unistd.h>

static volatile std::sig_atomic_t beenden = 0;  // das Einzige, was ein Handler setzen darf

extern "C" void handler(int sig) {
    beenden = sig;
}

int main(int argc, char**) {
    struct sigaction sa {};
    sa.sa_handler = handler;
    sigaction(SIGINT, &sa, nullptr);   // Strg+C
    sigaction(SIGTERM, &sa, nullptr);  // systemctl stop, kill
    sigaction(SIGALRM, &sa, nullptr);
    if (argc == 1) alarm(3);  // ohne Argument: nach 3 s kommt SIGALRM

    std::cout << "Dienst läuft (PID " << getpid() << ") – Strg+C beendet" << std::endl;
    int runden = 0;
    while (!beenden) {
        pause();  // schläft, bis ein Signal kommt
        runden++;
    }
    const char* name = beenden == SIGALRM   ? "SIGALRM"
                       : beenden == SIGTERM ? "SIGTERM"
                                            : "SIGINT";
    std::cout << "Signal " << beenden << " (" << name << ") nach " << runden
              << " Runde(n): aufräumen und beenden\n";
    return 0;
}
