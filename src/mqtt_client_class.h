//
// Created by kylerk on 11/20/2025.
//
#pragma once
#include "../src/weather_data.h"

#ifndef MQTT_CLIENT_CLASS_H
#define MQTT_CLIENT_CLASS_H

class MQTTClientBase {
    struct mosquitto *mqtt_client = nullptr;
    std::string brokerHost;
    int brokerPort = 0;
    std::string client_id;
    std::vector<std::string> receivedMessages;
    std::atomic<bool> messageReceived{false};
    std::atomic<bool> connected{false};


    static void onConnectCallback(mosquitto *mosq, void *obj, int rc) {
        auto *client = static_cast<MQTTClientBase*>(obj);
        if (rc == 0) {
            client->connected = true;
            std::cout << "[WeatherData] Connected to broker" << std::endl;

            mosquitto_subscribe(mosq, nullptr, "device1/responses", 0);
        } else {
            client->connected = false;
            std::cerr << "[WeatherData] Connection failed: " << rc << std::endl;
        }
    }

    static void onMessageCallback(struct mosquitto *mosq, void *obj,
                                    const struct mosquitto_message *message) {
        auto *client = static_cast<MQTTClientBase*>(obj);

        std::string msg(static_cast<char*>(message->payload), message->payloadlen);
        client->receivedMessages.push_back(msg);
        client->messageReceived = true;
        std::cout << "[WeatherData] Received: " << msg << std::endl;
    }

public:
    virtual ~MQTTClientBase() {
        disconnectBroker();
    }
    explicit MQTTClientBase(const char * str) : client_id(str) {
        const char* host_env = std::getenv("MQTT_BROKER_HOST");
        brokerHost = host_env ? host_env : "mqtt-broker";

        const char* port_env = std::getenv("MQTT_BROKER_PORT");
        brokerPort = port_env ? std::stoi(port_env) : 1883;

        std::cout << "[MQTT] Client Configured For: " << brokerHost
                  << ":" << brokerPort << std::endl;
    }

    virtual bool connectBroker(const std::string &clientName, void *obj) {
        if (connected) {
            std::cout << "[MQTT:" << client_id << "] Already connected" << std::endl;
            return true;
        }

        mosquitto_lib_init();

        mqtt_client = mosquitto_new(client_id.c_str(), true, this);
        if (!mqtt_client) {
            std::cerr << "[MQTT:" << client_id << "] Failed to create client" << std::endl;
            return false;
        }

        mosquitto_connect_callback_set(mqtt_client, onConnectCallback);
        mosquitto_message_callback_set(mqtt_client, onMessageCallback);

        int rc = mosquitto_connect(mqtt_client, brokerHost.c_str(), brokerPort, 60);
        if (rc != MOSQ_ERR_SUCCESS)
        {
            std::cerr << "[MQTT:" << client_id << "] Connect failed: "
            << mosquitto_strerror(rc) << std::endl;
            return false;
        }

        mosquitto_loop_start(mqtt_client);

        for (int i = 0; i < 50 && !connected; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return connected;
    }

    bool disconnectBroker() { // TODO

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
    std::string getLastMessage() {

        return "No data in queue!\n";
    }

    static std::vector<WeatherData> queryReadingsByID(int id) {
        std::vector<WeatherData> results;
        const WeatherData wd;
        results.push_back(wd);
        return results;
    }

};
#endif