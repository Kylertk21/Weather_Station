//
// Created by kylerk on 11/20/2025.
//
#pragma once
#include "../src/weather_data.h"

#ifndef MQTT_CLIENT_CLASS_H
#define MQTT_CLIENT_CLASS_H

class MQTTClientBase {
    std::string broker_host;
    int broker_port = 0;
    std::atomic<bool> message_received{false};
    std::vector<std::string> received_messages;
    std::string client_id;

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
            std::cerr << "[WeatherData] Connection failed: " << rc << std::endl;
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
    static struct mosquitto* mqttClient;
    virtual ~MQTTClientBase() = default;
    explicit MQTTClientBase(const char * str) : client_id(str) {
        const char* host_env = std::getenv("MQTT_BROKER_HOST");
        broker_host = host_env ? host_env : "mqtt-broker";

        const char* port_env = std::getenv("MQTT_BROKER_PORT");
        broker_port = port_env ? std::stoi(port_env) : 1883;

        std::cout << "[MQTT] Client Configured For: " << broker_host
                  << ":" << broker_port << std::endl;
    }

    static bool connectBroker(const std::string &clientName) {
        if (brokerConnected) {
            std::cout << "[WeatherData] Already Connected" << std::endl;
            return true;
        }

        const char* hostEnv = std::getenv("MQTT_BROKER_HOST");
        std::string brokerHost = hostEnv ? hostEnv : "mqtt-broker";

        const char* portEnv = std::getenv("MQTT_BROKER_PORT");
        int brokerPort = portEnv ? std::stoi(portEnv) : 1883;

        mosquitto_lib_init();

        mqttClient = mosquitto_new(clientName.c_str(), true, nullptr); // delete messages and sessions on disconnect
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

    bool disconnectBroker() {

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