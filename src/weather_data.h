//
// Created by kylerk on 11/10/2025.
//
#pragma once
#include <atomic>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <utility>
#include <ctime>
using namespace std;

#ifndef WEATHER_STATION_DASHBOARD_DATA_H
#define WEATHER_STATION_DASHBOARD_DATA_H

class WeatherData {

public:
    WeatherData() {
        topic = "";
        temperature = 0;
        pressure = 0.0;
        humidity = 0.0;
        rain = 0.0;
        wind = 0.0;
        isValid = false;
        timestamp = time(nullptr);

    }
    void setData(const string& top, const int temp, const float press,
                  const float humid, const float ra, const float wi) {

        this->topic = top;
        this->temperature = temp;
        this->pressure = press;
        this->humidity = humid;
        this->rain = ra;
        this->wind = wi;

    }

    void populateData() {
        dataMap["topic"] = topic;
        dataMap["temperature"] = std::to_string(temperature);
        dataMap["pressure"] = std::to_string(pressure);
        dataMap["humidity"] = std::to_string(humidity);
        dataMap["rain"] = std::to_string(rain);
        dataMap["wind"] = std::to_string(wind);
    }

    [[nodiscard]] string getTopic() const {
        return topic;
    }
    [[nodiscard]] int getTemperature() const {
        return temperature;
    }
    [[nodiscard]] float getPressure() const {
        return pressure;
    }
    [[nodiscard]] float getHumidity() const {
        return humidity;
    }
    [[nodiscard]] float getRain() const {
        return rain;
    }
    [[nodiscard]] float getWind() const {
        return wind;
    }

    static bool connectBroker() {

        return false;
    }

    static bool requestData(const string& request) {

        return false;
    }

    string receiveData() {

        return "No data received";
    }


    bool validateData(const string& data) {
        this->isValid = false;
        return isValid;
    }

private:
    string topic;
    int temperature;
    float pressure;
    float humidity;
    float rain;
    float wind;
    time_t timestamp{};
    std::atomic<bool> isValid;
    std::unordered_map<string, string> dataMap;

    WeatherData process_data();
};

// ========================================================================================
// MQTT Client Class
// ========================================================================================

class MQTT_Client {
    struct mosquitto *client = nullptr;
    std::string broker_host;
    int broker_port = 0;
    atomic<bool> connected{false};
    std::atomic<bool> message_received{false};
    std::vector<std::string> received_messages;
    std::string client_id;

public:
    MQTT_Client() = default;
    explicit MQTT_Client(const char * str) : client_id(str) {
        const char* host_env = std::getenv("MQTT_BROKER_HOST");
        broker_host = host_env ? host_env : "mqtt-broker";

        const char* port_env = std::getenv("MQTT_BROKER_PORT");
        broker_port = port_env ? std::stoi(port_env) : 1883;

        std::cout << "[MQTT] Client Configured For: " << broker_host
                  << ":" << broker_port << std::endl;
    }

    bool connect() {

        return false;
    }
    bool disconnect() {

        return false;
    }
    bool isConnected() {
        return connected;
    }
    bool publish(const std::string topic, std::string payload) {

        return false;
    }
    bool subscribe(std::string topic) {

        return false;
    }
    std::string getLastMessage() {

        return "No data in queue!\n";
    }



};

/*

class MQTT_Test_Client {
private:
    struct mosquitto *client = nullptr;
    std::string broker_host;
    int broker_port = 0;
    std::atomic<bool> connected{false};
    std::atomic<bool> message_received{false};
    std::vector<std::string> received_messages;
    std::string client_id;


    static void on_connect_callback(struct mosquitto *mosq, void *obj, int rc) {
        auto *test_client = static_cast<MQTT_Test_Client*>(obj);
        test_client->connected = (rc == 0);

        if (rc == 0) {
            std::cout << "[MQTT] Connected Successfully To Broker" << std::endl;
        } else {
            std::cerr << "[MQTT] Connection Failed!: " << rc << std::endl;
        }
    }

    static void on_message_callback(struct mosquitto *mosq, void *obj,
                                    const struct mosquitto_message *message) {
        auto *test_client = static_cast<MQTT_Test_Client*>(obj);
        const std::string msg(static_cast<char*>(message->payload), message->payloadlen);
        test_client->received_messages.push_back(msg);
        test_client->message_received = true;

        std::cout << "[MQTT] Received Message on Topic: '" << message->topic
                  << "': " << msg << std::endl;
    }

    static void on_subscribe_callback(struct mosquitto *mosq, void *o){}


public:
    MQTT_Test_Client(const std::string& id = "test-client") : client_id(id) {
        const char* host_env = std::getenv("MQTT_BROKER_HOST");
        broker_host = host_env ? host_env : "mqtt-broker";

        const char* port_env = std::getenv("MQTT_BROKER_PORT");
        broker_port = port_env ? std::stoi(port_env) : 1883;

        std::cout << "[MQTT] Configured for " << broker_host << ":" << broker_port << std::endl;
    }

};
 */




#endif //WEATHER_STATION_DASHBOARD_DATA_H