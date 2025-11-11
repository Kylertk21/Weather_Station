//
// Created by kylerk on 11/10/2025.
//
#include <gtest/gtest.h>
#include "../src/weather_data.h"
#include "crow.h"

class WeatherDataTest : public testing::Test {
protected:
    Weather_Data data;

    void SetUp() override {
        data.set_data(
            "device1",
            72,
            1013.2,
            45.5,
            0.1,
            5.4 );
    }
    void TearDown() override {

    }

};

TEST_F(WeatherDataTest, TestSetData) {

    EXPECT_EQ(data.get_temperature(), 72);
    EXPECT_FLOAT_EQ(data.get_pressure(), 1013.2);
    EXPECT_FLOAT_EQ(data.get_humidity(), 45.5);
    EXPECT_FLOAT_EQ(data.get_rain(), 0.1);
    EXPECT_FLOAT_EQ(data.get_wind(), 5.4);

}

class CrowAppTest : public testing::Test {
protected:
    crow::SimpleApp app;

    void SetUp() override {
        CROW_ROUTE(app, "/")([]() {
            crow::mustache::set_base("../src/templates");
            auto page = crow::mustache::load_text("index.html");
            return page;
        });
    }
};

TEST_F(CrowAppTest, TestIndexRoute) {
    crow::request req;
    req.url = "/";
    req.method = "GET"_method;

    crow::response res;

    // Properly initialize a routing_handle_result for the new API
    std::unique_ptr<crow::routing_handle_result> route_result = std::make_unique<crow::routing_handle_result>();
    app.handle(req, res, route_result);

    EXPECT_EQ(res.code, 200);
    EXPECT_NE(res.body.find("Weather Station Dashboard"), std::string::npos);
}


