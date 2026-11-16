// Kapitel 20: haus_bruecke – ein ROS-2-Knoten als Brücke zwischen Roboter und Haus
//   Haus → Roboter:  /haus/<Item>/zustand   (std_msgs/String, letzter Wert bleibt)
//   Roboter → Haus:  /haus/<Item>/befehl    (std_msgs/String) → Command an openHAB
//   Roboter → Haus:  /roboter/status, /roboter/akku → Items Roboter_Status, Roboter_Akku
#include <openhab/openhab.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <sstream>
#include <std_msgs/msg/float32.hpp>
#include <std_msgs/msg/string.hpp>
#include <string>
#include <thread>
#include <vector>

using std_msgs::msg::Float32;
using std_msgs::msg::String;

class HausBruecke : public rclcpp::Node {
public:
    HausBruecke()
        : Node("haus_bruecke"),
          client_(umgebung("OPENHAB_URL", "http://127.0.0.1:8080"), std::nullopt,
                  std::nullopt, umgebung("OPENHAB_TOKEN", "")),
          items_(client_) {
        auto namen =
            declare_parameter<std::vector<std::string>>("items", {"Wohnzimmer_Licht"});
        if (!client_.isLoggedIn())
            throw std::runtime_error("openHAB nicht erreichbar: " + client_.lastError());
        robotItemsAnlegen();

        // Zustände: "transient local" – wer später abonniert, bekommt den letzten Wert
        auto qos = rclcpp::QoS(1).transient_local();
        std::string topics;
        for (const std::string& item : namen) {
            zustand_[item] = create_publisher<String>("haus/" + item + "/zustand", qos);
            befehl_.push_back(create_subscription<String>(
                "haus/" + item + "/befehl", 10,
                [this, item](const String& m) { befehlSenden(item, m.data); }));
            veroeffentlichen(item, items_.getItemState(item));  // Startwert
            topics +=
                (topics.empty() ? "" : ",") + ("openhab/items/" + item + "/statechanged");
        }
        // Roboter → Haus: Philosophie 2, der Roboter ist die Quelle der Wahrheit
        status_ = create_subscription<String>(
            "roboter/status", 10,
            [this](const String& m) { melden("Roboter_Status", m.data); });
        akku_ = create_subscription<Float32>("roboter/akku", 10, [this](const Float32& m) {
            std::ostringstream wert;
            wert << m.data << " %";
            melden("Roboter_Akku", wert.str());
        });

        strom_ = openhab::Events(client_).getEvents(topics);
        if (strom_.isError()) throw std::runtime_error(strom_.error());
        leser_ = std::thread([this] {  // Ereignisse von openHAB → ROS-Topics
            for (const openhab::json& e : strom_) {
                std::string topic = e["topic"];
                std::string item = topic.substr(14, topic.find('/', 14) - 14);
                veroeffentlichen(item, e["payload"]["value"]);
            }
        });
        RCLCPP_INFO(get_logger(), "Brücke zu %s steht: %zu Items", client_.url().c_str(),
                    namen.size());
    }

    ~HausBruecke() override {
        strom_.interrupt();  // Leser aufwecken
        if (leser_.joinable()) leser_.join();
    }

private:
    static std::string umgebung(const char* name, const char* standard) {
        const char* wert = std::getenv(name);
        return wert ? wert : standard;
    }

    void robotItemsAnlegen() {
        openhab::json liste = {
            {{"type", "String"}, {"name", "Roboter_Status"}, {"label", "Roboter"}},
            {{"type", "Number:Dimensionless"},
             {"name", "Roboter_Akku"},
             {"label", "Akku"}}};
        openhab::json r = items_.addOrUpdateItems(liste);
        if (openhab::isError(r))
            RCLCPP_WARN(get_logger(), "Items: %s", openhab::errorMessage(r).c_str());
        items_.addMetadata("Roboter_Akku", "unit", {{"value", "%"}});  // sonst 0.875
    }

    void veroeffentlichen(const std::string& item, const openhab::json& state) {
        auto p = zustand_.find(item);
        if (p == zustand_.end() || !state.is_string()) return;
        String m;
        m.data = state.get<std::string>();
        p->second->publish(m);  // publish() ist thread-sicher
        RCLCPP_INFO(get_logger(), "Haus → Roboter: %s = %s", item.c_str(), m.data.c_str());
    }

    void befehlSenden(const std::string& item, const std::string& befehl) {
        openhab::json r = items_.sendCommand(item, befehl, "ros2");
        if (openhab::isError(r))
            RCLCPP_ERROR(get_logger(), "%s: %s", item.c_str(),
                         openhab::errorMessage(r).c_str());
        else
            RCLCPP_INFO(get_logger(), "Roboter → Haus: %s <- %s", item.c_str(),
                        befehl.c_str());
    }

    void melden(const std::string& item, const std::string& state) {
        openhab::json r = items_.postUpdate(item, state, "ros2");
        if (openhab::isError(r))
            RCLCPP_ERROR(get_logger(), "%s: %s", item.c_str(),
                         openhab::errorMessage(r).c_str());
        else
            RCLCPP_INFO(get_logger(), "Roboter → Haus: %s = %s", item.c_str(),
                        state.c_str());
    }

    openhab::OpenHABClient client_;
    openhab::Items items_;
    std::map<std::string, rclcpp::Publisher<String>::SharedPtr> zustand_;
    std::vector<rclcpp::Subscription<String>::SharedPtr> befehl_;
    rclcpp::Subscription<String>::SharedPtr status_;
    rclcpp::Subscription<Float32>::SharedPtr akku_;
    openhab::EventStream strom_;
    std::thread leser_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    try {
        rclcpp::spin(std::make_shared<HausBruecke>());  // Strg+C beendet spin()
    } catch (const std::exception& e) {
        std::cerr << "haus_bruecke: " << e.what() << "\n";
        rclcpp::shutdown();
        return 1;
    }
    rclcpp::shutdown();
    return 0;
}
