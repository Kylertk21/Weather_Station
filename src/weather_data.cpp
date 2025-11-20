//
// Created by kylerk on 11/20/2025.
//

#include "weather_data.h"

std::atomic<bool> WeatherData::brokerConnected{false};
std::atomic<bool> WeatherData::messageReady{false};
std::string WeatherData::lastReceivedMessage{};
struct mosquitto *WeatherData::mqttClient = nullptr;