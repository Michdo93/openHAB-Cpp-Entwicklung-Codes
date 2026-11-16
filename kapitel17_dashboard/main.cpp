// Kapitel 17: ohdash – ein Dashboard für openHAB nach Model-View-Controller
//   ohdash [--gruppe=gWohnzimmer] [--text [--dauer=Sekunden]] [--bild=datei.png]
#include <openhab/openhab.hpp>

#include <QApplication>
#include <QTimer>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "ansicht_qt.hpp"
#include "ansicht_text.hpp"
#include "controller.hpp"
#include "model.hpp"

int main(int argc, char** argv) {
    QString gruppe = "gWohnzimmer", bild;
    bool text = false;
    int dauer = 30;
    for (int i = 1; i < argc; i++) {
        QString arg = argv[i];
        if (arg.startsWith("--gruppe="))
            gruppe = arg.mid(9);
        else if (arg == "--text")
            text = true;
        else if (arg.startsWith("--dauer="))
            dauer = arg.mid(8).toInt();
        else if (arg.startsWith("--bild="))
            bild = arg.mid(7);
    }
    // Die Text-View braucht keine Fensterumgebung: QCoreApplication genügt
    std::unique_ptr<QCoreApplication> app =
        text ? std::make_unique<QCoreApplication>(argc, argv)
             : std::make_unique<QApplication>(argc, argv);

    const char* url = std::getenv("OPENHAB_URL");
    const char* token = std::getenv("OPENHAB_TOKEN");
    openhab::OpenHABClient client(url ? url : "http://127.0.0.1:8080", std::nullopt,
                                  std::nullopt, token ? token : "");
    if (!client.isLoggedIn()) {
        std::cerr << "keine Verbindung: " << client.lastError() << "\n";
        return 1;
    }

    Model model;                           // M
    Controller controller(client, model);  // C
    if (controller.laden(gruppe) < 0 || !controller.starten()) {
        std::cerr << gruppe.toStdString() << ": "
                  << controller.letzterFehler().toStdString() << "\n";
        return 1;
    }
    std::cout << model.anzahl() << " Items aus " << gruppe.toStdString() << " geladen\n";

    if (text) {  // V – die gewählte Ansicht
        TextAnsicht ansicht(model, controller, dauer);
        int rc = app->exec();
        std::cout << "Endstand:\n";
        ansicht.tabelle();
        return rc;
    }
    QtAnsicht fenster(model, controller, gruppe);
    fenster.show();
    if (!bild.isEmpty())  // Bildschirmfoto nach 3 s, dann beenden
        QTimer::singleShot(3000, &fenster, [&] {
            fenster.grab().save(bild);
            app->quit();
        });
    return app->exec();
}
