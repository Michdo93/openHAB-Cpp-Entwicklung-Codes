// Kapitel 9: Fehlersuche – drei Speicherfehler für AddressSanitizer und valgrind
//   cmake -S . -B build-asan -DMIT_ASAN=ON && cmake --build build-asan -j2
//   build-asan/kapitel09_werkzeuge/04_fehlersuche ueberlauf
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

static void ueberlauf() {
    std::vector<int> werte = {21, 22, 23};
    std::cout << werte[3] << "\n";  // [] prüft nicht – .at(3) würde werfen
}

static void nachDelete() {
    int* helligkeit = new int(40);
    delete helligkeit;
    std::cout << *helligkeit << "\n";  // Zugriff auf freigegebenen Speicher
}

static void leck() {
    auto* kopie = new std::string("Kopie von Haus_Status");  // nie freigegeben
    std::cout << *kopie << "\n";
}

int main(int argc, char** argv) {
    std::string fall = argc > 1 ? argv[1] : "";
    if (fall == "ueberlauf")
        ueberlauf();
    else if (fall == "nach_delete")
        nachDelete();
    else if (fall == "leck")
        leck();
    else {
        std::cerr << "Aufruf: " << argv[0] << " ueberlauf | nach_delete | leck\n";
        return 2;
    }
    return 0;
}
