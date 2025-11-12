//
// Created by kylerk on 11/10/2025.
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/weather_data.h"
#include "crow.h"

class IJsonProvider {
public:
    virtual ~IJsonProvider() = default;
    virtual std::string getJson() const = 0;
};

class MockJsonProvider : public IJsonProvider {
public:
    MOCK_METHOD(std::string, getJson, (), (const, override));
};


class WeatherDataTest : public testing::Test {
protected:
    Weather_Data data;
    MockJsonProvider mock;

    void SetUp() override {
        data.set_data(
            "device1",
            72,
            1013.2,
            45.5,
            0.1,
            5.4 );
        ON_CALL(mock, getJson())
            .WillByDefault(testing::Return(
                R"({
                        "temp":72,
                        "pressure":1013.2,
                        "humidity":45.5,
                        "rain":0.1,
                        "wind":5.4
                                    })"));
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
            crow::mustache::set_base("src/templates");
            auto page = crow::mustache::load_text("index.html");
            return page;
        });
        app.validate();
    }
};

TEST_F(CrowAppTest, TestIndexRoute) {
    crow::request req;
    req.url = "/";
    req.method = "GET"_method;

    crow::response res;

    app.handle_full(req, res);

    ASSERT_GT(res.code, 0) << "Crow response not initialized — app may not be validated";
    ASSERT_FALSE(res.body.empty()) << "Crow returned empty body";

    EXPECT_EQ(res.code, 200);
    EXPECT_NE(res.body.find("Weather Station Dashboard"), std::string::npos);
}

TEST_F(CrowAppTest, TestDataUpdate) {
    crow::request req;
    req.url = "/";
    req.method = "GET"_method;

    crow::response res;

    std::unique_ptr<crow::routing_handle_result> route_result = std::make_unique<crow::routing_handle_result>();
    app.handle(req, res, route_result);

    EXPECT_EQ(res.code, 200);

    auto json_response = crow::json::load(res.body);
    ASSERT_TRUE(json_response);

    EXPECT_EQ(json_response["device"], "device1");
    EXPECT_EQ(json_response["temperature"].i(), 72);
    EXPECT_FLOAT_EQ(json_response["pressure"].d(), 1013.2);
    EXPECT_FLOAT_EQ(json_response["humidity"].d(), 45.5);
    EXPECT_FLOAT_EQ(json_response["rain"].d(), 0.1);
    EXPECT_FLOAT_EQ(json_response["wind"].d(), 5.4);
}

TEST_F(CrowAppTest, TestDataRequestSent) {
    GTEST_SKIP() << "Not implemented...";
}

TEST_F(CrowAppTest, TestDataRequestFromTimePeriodSent) {
    GTEST_SKIP() << "Not implemented...";
}

TEST_F(CrowAppTest, TestServerQueriesDataBase) {
    GTEST_SKIP() << "Not implemented...";
}






