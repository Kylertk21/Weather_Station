//
// Created by kylerk on 11/12/2025.
//
#ifndef WEATHER_STATION_ROUTES_H
#define WEATHER_STATION_ROUTES_H

#pragma once
#include "crow.h"

inline crow::SimpleApp create_app() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
      crow::mustache::set_base("src/templates");
      auto page = crow::mustache::load_text("index.html");
      return page;
    });
    return app;
}

#endif //WEATHER_STATION_ROUTES_H