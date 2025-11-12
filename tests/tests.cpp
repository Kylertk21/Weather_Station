//
// Created by kylerk on 11/10/2025.
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/weather_data.h"
#include "../src/routes.h"

class IJsonProvider {
public:
    virtual ~IJsonProvider() = default;
    virtual std::string getJson() const = 0;
};

class MockJsonProvider final : public IJsonProvider {
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

TEST_F(WeatherDataTest, TestSetData) { // Test data is set in class structure

    EXPECT_EQ(data.get_temperature(), 72);
    EXPECT_FLOAT_EQ(data.get_pressure(), 1013.2);
    EXPECT_FLOAT_EQ(data.get_humidity(), 45.5);
    EXPECT_FLOAT_EQ(data.get_rain(), 0.1);
    EXPECT_FLOAT_EQ(data.get_wind(), 5.4);

}

TEST_F(WeatherDataTest, TestSetDataEMPTY) {

    EXPECT_CALL(mock, getJson())
        .WillOnce(testing::Return(""));

    const std::string json = mock.getJson();
    EXPECT_FALSE(data.isJson(json));
}

TEST_F(WeatherDataTest, TestDataInJSONFormat) { // Data in JSON format
    GTEST_SKIP() << "Not implemented...";
}

TEST_F(WeatherDataTest, TestDataNOTInJSONFormat) { // Data NOT in JSON format
    GTEST_SKIP() << "Not implemented...";
}

TEST(BrokerTest, TestReadData) { // Test data can be read from /device/responses
    GTEST_SKIP() << "Not implemented...";
}

TEST(BrokerTest, TestReadDataFAIL) { // Test read data FAIL
    GTEST_SKIP() << "Not implemented...";
}

TEST(BrokerTest, TestSendData) { // Test data can be sent to /device/requests
    GTEST_SKIP() << "Not implemented...";
}

TEST(BrokerTest, TestSendDataFAIL) { // Test send data FAIL
    GTEST_SKIP() << "Not implemented";
}


class CrowAppTest : public testing::Test {
protected:
    crow::SimpleApp app{create_app()};
    crow::request req;
    crow::response res;

};

TEST_F(CrowAppTest, TestIndexRoute) { // Test index page available
    req.url = "/";
    req.method = "GET"_method;

    app.handle_full(req, res);

    ASSERT_GT(res.code, 0) << "Crow response not initialized — app may not be validated";
    ASSERT_FALSE(res.body.empty()) << "Crow returned empty body";

    EXPECT_EQ(res.code, 200);
    EXPECT_NE(res.body.find("Weather Station Dashboard"), std::string::npos);
}

TEST_F(CrowAppTest, TestIndexRouteFAIL) { // Index page not available
    GTEST_SKIP() << "Not implemented...";
}

TEST_F(CrowAppTest, TestDataUpdate) { // Test data retrieved from server is in json format
    req.url = "/";
    req.method = "GET"_method;

    app.handle_full(req, res);

    EXPECT_EQ(res.code, 200);

    const auto json_response = crow::json::load(res.body);
    ASSERT_TRUE(json_response);

    EXPECT_EQ(json_response["device"], "device1");
    EXPECT_EQ(json_response["temperature"].i(), 72);
    EXPECT_FLOAT_EQ(json_response["pressure"].d(), 1013.2);
    EXPECT_FLOAT_EQ(json_response["humidity"].d(), 45.5);
    EXPECT_FLOAT_EQ(json_response["rain"].d(), 0.1);
    EXPECT_FLOAT_EQ(json_response["wind"].d(), 5.4);
}

TEST_F(CrowAppTest, TestDataRequestSent) { // Test request for data sent from server
    GTEST_SKIP() << "Not implemented...";
}

TEST_F(CrowAppTest, TestDataRequestFromTimePeriodSent) { // Test request for data from time period sent from server
    GTEST_SKIP() << "Not implemented...";
}

TEST_F(CrowAppTest, TestServerQueriesDataBase) { // Test server queries database for data from time period
    GTEST_SKIP() << "Not implemented...";
}









