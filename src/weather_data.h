//
// Created by kylerk on 11/10/2025.
//
#include <iostream>
#include <string>
#include <utility>
using namespace std;

#ifndef WEATHER_STATION_DASHBOARD_DATA_H
#define WEATHER_STATION_DASHBOARD_DATA_H

class Weather_Data {
public:
    Weather_Data() = default;
    void receive_data();
    void set_data(const string& top, const int temp, const float press,
                  const float humid, const float ra, const float wi) {

        this->topic = std::move(topic);
        this->temperature = temp;
        this->pressure = press;
        this->humidity = humid;
        this->rain = ra;
        this->wind = wi;

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

    static bool isJson(string json) {
        return true;
    }

private:
    string topic;
    int temperature = 0;
    float pressure = 0.0;
    float humidity = 0.0;
    float rain = 0;
    float wind = 0.0;

    Weather_Data process_data();
};




#endif //WEATHER_STATION_DASHBOARD_DATA_H