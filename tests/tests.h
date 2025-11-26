//
// Created by kylerk on 11/20/2025.
//
#pragma once
#include "../src/mqtt_client_class.h"


#ifndef WEATHER_STATION_TESTS_H
#define WEATHER_STATION_TESTS_H



class MQTTGatewayClient final : public MQTTClientBase {
    static mosquitto *mqttClient;
    static std::atomic<bool> brokerConnected;
    static std::string lastReceivedMessage;
    static std::atomic<bool> messageReady;
    static std::atomic<bool> messageReceived;

public:
    MQTTGatewayClient(const char* name)
        : MQTTClientBase(name) {}

    bool start() {
        return connectBroker("test-gateway-client");
    }

    bool forwardData(const std::string& topic, const std::string& jsonData) {
        int rc = mosquitto_publish(
            mqttClient,
            nullptr,
            topic.c_str(),
            jsonData.size(),
            jsonData.c_str(),
            0,
            false
            );
        return MOSQ_ERR_SUCCESS;
    }

};
#endif //WEATHER_STATION_TESTS_H