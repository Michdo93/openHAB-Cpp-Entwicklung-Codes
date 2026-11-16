// Kapitel 18: Ein Schaltausgang
#include "ausgang.hpp"

#include <iostream>

#ifdef MIT_GPIOD
#include <gpiod.hpp>

// libgpiod 2: eine Leitung als Ausgang anfordern; RAII gibt sie wieder frei
class GpioAusgang : public Ausgang {
public:
    GpioAusgang(const std::string& chip, unsigned int leitung, const std::string& name)
        : leitung_(leitung),
          name_(name),
          anfrage_(gpiod::chip("/dev/" + chip)
                       .prepare_request()
                       .set_consumer("ohgeraet")
                       .add_line_settings(
                           leitung, gpiod::line_settings()
                                        .set_direction(gpiod::line::direction::OUTPUT)
                                        .set_output_value(gpiod::line::value::INACTIVE))
                       .do_request()) {}
    ~GpioAusgang() override { setzen(false); }  // beim Beenden ausschalten

    bool setzen(bool an) override {
        try {
            anfrage_.set_value(
                leitung_, an ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
        } catch (const std::exception& e) {
            std::cerr << name_ << ": " << e.what() << "\n";
            return false;
        }
        an_ = an;
        return true;
    }
    bool an() const override { return an_; }
    std::string name() const override { return name_; }

private:
    unsigned int leitung_;
    std::string name_;
    gpiod::line_request anfrage_;
    bool an_ = false;
};
#endif

std::unique_ptr<Ausgang> ausgangOeffnen(const std::string& beschreibung) {
    if (beschreibung == "attrappe") return std::make_unique<AttrappenAusgang>();
#ifdef MIT_GPIOD
    auto doppelpunkt = beschreibung.find(':');
    if (doppelpunkt != std::string::npos) {
        try {  // libgpiod meldet Fehler als Ausnahme
            return std::make_unique<GpioAusgang>(
                beschreibung.substr(0, doppelpunkt),
                std::stoul(beschreibung.substr(doppelpunkt + 1)), beschreibung);
        } catch (const std::exception& e) {
            std::cerr << beschreibung << ": " << e.what() << "\n";
            return nullptr;
        }
    }
#else
    std::cerr << beschreibung << ": ohne MIT_GPIOD übersetzt – nur \"attrappe\" möglich\n";
#endif
    return nullptr;
}
