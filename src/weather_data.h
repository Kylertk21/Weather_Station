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
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

using namespace std;

#ifndef WEATHER_STATION_DASHBOARD_DATA_H
#define WEATHER_STATION_DASHBOARD_DATA_H

class WeatherData {
    int data_ID;
    string topic;
    int temperature;
    float pressure;
    float humidity;
    float rain;
    float wind;
    time_t timestamp{};
    std::atomic<bool> isValid{};
    std::unordered_map<string, string> dataMap;

    WeatherData process_data();

public:
    WeatherData() {
        data_ID = 0;
        topic = "";
        temperature = 0;
        pressure = 0.0;
        humidity = 0.0;
        rain = 0.0;
        wind = 0.0;
        isValid = false;
        timestamp = time(nullptr);

    }
    void setData(const int i, const string& top, const int temp, const float press,
                  const float humid, const float ra, const float wi, const string &t) {
        this->data_ID = i;
        this->topic = top;
        this->temperature = temp;
        this->pressure = press;
        this->humidity = humid;
        this->rain = ra;
        this->wind = wi;

        // convert string t to time_t
        std::tm tm_struct = {};
        std::istringstream ss(t);
        ss >> std::get_time(&tm_struct, "%Y-%m-%d %H:%M:%S");

        if (ss.fail()) {
            std::cerr << "Error parsing time string" << std::endl;
        } else {
            this->timestamp = std::mktime(&tm_struct);
        }
    }

    void populateData() {
        dataMap["data ID"] = std::to_string(data_ID);
        dataMap["topic"] = topic;
        dataMap["temperature"] = std::to_string(temperature);
        dataMap["pressure"] = std::to_string(pressure);
        dataMap["humidity"] = std::to_string(humidity);
        dataMap["rain"] = std::to_string(rain);
        dataMap["wind"] = std::to_string(wind);
        dataMap["timestamp"] = std::to_string(timestamp);
    }

    [[nodiscard]] int getDataID() const {
        return data_ID;
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
    [[nodiscard]] time_t getTimeStamp() const {
        return timestamp;
    }

    static string convertTime (const time_t time) {
        std::tm tm{};
        localtime_r(&time, &tm);

        std::ostringstream oss;

        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
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
};

// ========================================================================================
// Weather Database
// ========================================================================================

class WeatherDataBase {
    string host;
    int port{};
    int dataCount{};
    string db_name;
    string db_user;
    string db_pass;
    atomic<bool> connected = false;
    atomic<bool> committed = false;

public:
    WeatherDataBase() = default;
    WeatherDataBase(const char * h, int pt, const char * n, const char * u, const char * ps) : port(5432) {
        host = h;
        port = pt;
        dataCount = 0;
        db_name = n;
        db_user = u;
        db_pass = ps;
    }

    bool connect() {

        return connected;
    }

    bool disconnect() {

        return connected;
    }

    bool isConnected() {

        return connected;
    }

    bool commitReading(const WeatherData & data) {

         return committed;
    }

    static void clearAllReadings() {

    }

    int getDataCount(const char * str) const {

        return dataCount;
    }
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


#endif //WEATHER_STATION_DASHBOARD_DATA_H