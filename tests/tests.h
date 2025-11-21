//
// Created by kylerk on 11/20/2025.
//
#pragma once
#include "../src/mqtt_client_class.h"


#ifndef WEATHER_STATION_TESTS_H
#define WEATHER_STATION_TESTS_H



class MQTTGatewayClient : public MQTTClientBase {

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