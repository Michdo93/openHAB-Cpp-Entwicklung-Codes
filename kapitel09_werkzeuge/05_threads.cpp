// Kapitel 9: std::thread, mutex, condition_variable, atomic und async
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

std::mutex sperre;            // schützt die Warteschlange
std::condition_variable neu;  // weckt den Verbraucher
std::queue<std::string> ereignisse;
bool fertig = false;

void erzeuger() {
    for (const char* e : {"Licht ON", "Dimmer 40", "Tür OPEN"}) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::lock_guard<std::mutex> l(sperre);
            ereignisse.push(e);
        }  // Sperre freigeben, dann wecken
        neu.notify_one();
    }
    std::lock_guard<std::mutex> l(sperre);
    fertig = true;
    neu.notify_one();
}

int main() {
    std::thread t(erzeuger);
    for (;;) {
        std::unique_lock<std::mutex> l(sperre);
        neu.wait(l, [] { return !ereignisse.empty() || fertig; });  // ohne Dauerschleife
        if (ereignisse.empty()) break;
        std::string e = ereignisse.front();
        ereignisse.pop();
        l.unlock();
        std::cout << "verarbeitet: " << e << "\n";
    }
    t.join();  // auf das Ende des Threads warten

    std::atomic<int> zaehler{0};  // ohne Sperre threadsicher
    std::thread a([&] {
        for (int i = 0; i < 10000; i++) zaehler++;
    });
    std::thread b([&] {
        for (int i = 0; i < 10000; i++) zaehler++;
    });
    a.join();
    b.join();
    std::cout << "Zähler: " << zaehler << "\n";

    // async: eine Aufgabe im Hintergrund, das Ergebnis kommt als future
    std::future<double> mittel = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return (21.5 + 19.0 + 23.0) / 3;
    });
    std::cout << "Mittelwert: " << mittel.get() << " °C\n";  // get() wartet
    return 0;
}
