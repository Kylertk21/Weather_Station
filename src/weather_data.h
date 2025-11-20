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
#include <nlohmann/json.hpp>
#include <mosquitto.h>
#include <bits/this_thread_sleep.h>

using json = nlohmann::json;
using namespace std;

#ifndef WEATHER_STATION_DASHBOARD_DATA_H
#define WEATHER_STATION_DASHBOARD_DATA_H

class WeatherData {
    int dataID;
    string topic;
    int temperature;
    float pressure;
    float humidity;
    float rain;
    float wind;
    time_t timestamp{};
    bool isValid{};
    std::unordered_map<string, string> dataMap;

    WeatherData process_data();

    static struct mosquitto* mqttClient;
    static std::atomic<bool> brokerConnected;
    static std::string lastReceivedMessage;
    static std::atomic<bool> messageReady;
    static std::atomic<bool> messageReceived;


    static void onConnectCallback(mosquitto *mosq, void *obj, int rc) {
        if (rc == 0) {
            brokerConnected = true;
            std::cout << "[WeatherData] Connected to broker" << std::endl;

            mosquitto_subscribe(mosq, nullptr, "device1/responses", 0);
        } else {
            brokerConnected = false;
            std::cerr << "[WeatherData] Connection faled: " << rc << std::endl;
        }
    }

    static void onMessageCallback(struct mosquitto *mosq, void *obj,
                                    const struct mosquitto_message *message) {

        std::string msg(static_cast<char*>(message->payload), message->payloadlen);
        lastReceivedMessage = msg;
        messageReady = true;
        std::cout << "[WeatherData] Received: " << msg << std::endl;
    }

public:
    WeatherData() :
        dataID(0), topic(""), temperature(0), pressure(0.0),
        humidity(0.0), rain(0.0), wind(0.0) {}
    WeatherData(const WeatherData&) = default;

    void setData(const int i, const string& top, const int temp, const float press,
                 const float humid, const float ra, const float wi, const string &t) {

        this->dataID = i;
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
            std::cerr << "Timestamp not valid!" << std::endl;
            this->isValid = false;
        } else {
            this->timestamp = std::mktime(&tm_struct);
        }
        this->isValid = validateData();
    }


    bool validateData() const {
        bool validate = true;
        if (dataID < 1) {
            cerr << "Data ID cannot be less than 1!" << std::endl;
            validate = false;
        }
        if (topic == "") {
            cerr << "Topic cannot be blank!" << std::endl;
            validate = false;
        }
        if (temperature < -50 || temperature > 110) {
            cerr << "Temperature out of range!" << std::endl;
            validate = false;
        }
        if (pressure < 600.00 || pressure > 2000) {
            cerr << "Pressure out of range!" << std::endl;
            validate = false;
        }
        if (humidity < 0.0 || humidity > 100.0) {
            cerr << "Humidity out of range!" << std::endl;
            validate = false;
        }
        if (rain < 0.0 || rain > 100.0) {
            cerr << "Rain out of range!" << std::endl;
            validate = false;
        }
        if (wind < 0.0 || wind > 150.0) {
            cerr << "Wind out of range!" << std::endl;
            validate = false;
        }
        return validate;
    }

    static bool validateJSON(const std::string& jsonStr) {
        json j;

        try {
            j = json::parse(jsonStr);
        } catch (...) {
            std::cerr << "Invalid JSON format!" << std::endl;
            return false;
        }

        bool valid = true;


        auto require = [&](auto field, auto check, const std::string& msg) {
            if (!j.contains(field) || !check(j[field])) {
                std::cerr << msg << std::endl;
                valid = false;
            }
        };

        require("data_ID",     [](auto x){ return x.is_number_integer(); }, "Data ID JSON invalid!");
        require("topic",       [](auto x){ return x.is_string(); },         "Topic JSON invalid!");
        require("temperature", [](auto x){ return x.is_number(); },         "Temperature JSON invalid!");
        require("pressure",    [](auto x){ return x.is_number(); },         "Pressure JSON invalid!");
        require("humidity",    [](auto x){ return x.is_number(); },         "Humidity JSON invalid!");
        require("rain",        [](auto x){ return x.is_number(); },         "Rain JSON invalid!");
        require("wind",        [](auto x){ return x.is_number(); },         "Wind JSON invalid!");
        require("timestamp",   [](auto x){ return x.is_string(); },         "Timestamp JSON invalid!");

        return valid;
    }

    void populateDataToMap() {
        dataMap["data ID"] = std::to_string(dataID);
        dataMap["topic"] = topic;
        dataMap["temperature"] = std::to_string(temperature);
        dataMap["pressure"] = std::to_string(pressure);
        dataMap["humidity"] = std::to_string(humidity);
        dataMap["rain"] = std::to_string(rain);
        dataMap["wind"] = std::to_string(wind);
        dataMap["timestamp"] = std::to_string(timestamp);
    }

    [[nodiscard]] int getDataID() const {
        return dataID;
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
        if (brokerConnected) {
            std::cout << "[WeatherData] Already Connected" << std::endl;
            return true;
        }

        const char* hostEnv = std::getenv("MQTT_BROKER_HOST");
        std::string brokerHost = hostEnv ? hostEnv : "mqtt-broker";

        const char* portEnv = std::getenv("MQTT_BROKER_PORT");
        int brokerPort = portEnv ? std::stoi(portEnv) : 1883;

        mosquitto_lib_init();

        mqttClient = mosquitto_new("weather-data-client", true, nullptr); // delete messages and sessions on disconnect
        if (!mqttClient) {
            std::cerr << "[WeatherData] failed to create client" << std::endl;
            return false;
        }

        mosquitto_connect_callback_set(mqttClient, onConnectCallback);
        // Callback function: instance, user data, return code, called when connected to broker

        mosquitto_message_callback_set(mqttClient, onMessageCallback);
        // Callback function: instance, user data, message data, called when message received from broker

        int rc = mosquitto_connect(mqttClient, brokerHost.c_str(), brokerPort, 60);
        if (rc != MOSQ_ERR_SUCCESS) {
            std::cerr << "[WeatherData] Connect failed with: " << mosquitto_strerror(rc) << std::endl;
            return false;
        }
        return brokerConnected;
    }

    static bool waitForMessage(const int timeout_ms = 5000) {
        messageReceived = false;
        const auto start = std::chrono::steady_clock::now();

        while (!messageReceived) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start
                ).count();

            if (elapsed > timeout_ms) {
                std::cerr << "[MQTT_Client] Timeout waiting for message" << std::endl;
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return true;
    }

    static bool requestData(const string& request) {

        return false;
    }

    string receiveData(int waitTime) {

        return "No data received";
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

    bool present = false;
    bool committed = false;
    atomic<bool> connected = false;


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

        return this->connected;
    }

    bool disconnect() {

        return this->connected;
    }

    bool isConnected() {

        return this->connected;
    }

    bool commitReading(const WeatherData & data) {

         return this->committed;
    }

    bool isPresent() const {

        return this->present;
    }

    static WeatherData queryReadingByID(int i) {
        WeatherData query;
        return query;
    }

    static void clearAllReadings() {

    }

    int getDataCount(const char * str) const {

        return dataCount;
    }
};

#endif //WEATHER_STATION_DASHBOARD_DATA_H