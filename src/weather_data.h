//
// Created by kylerk on 11/10/2025.
//
#pragma once
#include <unordered_map>
#include <string>
#include <utility>
using namespace std;

#ifndef WEATHER_STATION_DASHBOARD_DATA_H
#define WEATHER_STATION_DASHBOARD_DATA_H

class Weather_Data {

public:
    Weather_Data() {
        topic = "";
        temperature = 0;
        pressure = 0.0;
        humidity = 0.0;
        rain = 0.0;
        wind = 0.0;
        isPopulated = false;
    }

    void receive_data();
    void set_data(const string& top, const int temp, const float press,
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
        dataMap["temperature"] = temperature;
        dataMap["pressure"] = pressure;
        dataMap["humidity"] = humidity;
        dataMap["rain"] = rain;
        dataMap["wind"] = wind;
    }

    [[nodiscard]] string get_topic() const {
        return topic;
    }
    [[nodiscard]] int get_temperature() const {
        return temperature;
    }
    [[nodiscard]] float get_pressure() const {
        return pressure;
    }
    [[nodiscard]] float get_humidity() const {
        return humidity;
    }
    [[nodiscard]] float get_rain() const {
        return rain;
    }
    [[nodiscard]] float get_wind() const {
        return wind;
    }

    static string readData() {
        return "";
    }

    bool validateData(string data) {
        this->isPopulated = false;
        return isPopulated;
    }

private:
    string topic;
    int temperature;
    float pressure;
    float humidity;
    float rain;
    float wind;
    bool isPopulated;

    std::unordered_map<string, string> dataMap;

    Weather_Data process_data();
};




#endif //WEATHER_STATION_DASHBOARD_DATA_H