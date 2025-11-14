//
// Created by kylerk on 11/10/2025.
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include "../src/weather_data.h"
#include "../src/routes.h"
#include <vector>
using json = nlohmann::json;

class IJsonProvider {
public:
    virtual ~IJsonProvider() = default;
    virtual std::string getJson() const = 0;
};

class MockJsonProvider final : public IJsonProvider {
public:
    MOCK_METHOD(std::string, getJson, (), (const, override));
};

// ========================================================================
// BASIC DATA TESTS
// ========================================================================

class WeatherDataTest : public testing::Test {
protected:
    WeatherData data = WeatherData();
    MockJsonProvider mock;

    void SetUp() override {
        data.setData(
            "device1",
            72,
            1013.2,
            45.5,
            0.1,
            5.4 );
        ON_CALL(mock, getJson())
            .WillByDefault(testing::Return(
                R"({
                        "topic":"device1/responses",
                        "temp":72,
                        "pressure":1013.2,
                        "humidity":45.5,
                        "rain":0.1,
                        "wind":5.4
                                    })"));
    }

};

class EmptyWeatherDataTest : public testing::Test {
protected:
    WeatherData emptyData = WeatherData();
    MockJsonProvider mock;

    void SetUp() override {
        emptyData.setData(
            "",
            0,
            0.0,
            0.0,
            0.0,
            0.0 );
        ON_CALL(mock, getJson())
            .WillByDefault(testing::Return(
                R"({})"));
    }
};

TEST_F(WeatherDataTest, TestSetData) { // Test data is set in class structure

    EXPECT_EQ(data.getTopic(), "device1/responses");
    EXPECT_EQ(data.getTemperature(), 72);
    EXPECT_FLOAT_EQ(data.getPressure(), 1013.2);
    EXPECT_FLOAT_EQ(data.getHumidity(), 45.5);
    EXPECT_FLOAT_EQ(data.getRain(), 0.1);
    EXPECT_FLOAT_EQ(data.getWind(), 5.4);

    const std::string json = mock.getJson();
    EXPECT_TRUE(data.validateData(json));

}

TEST_F(EmptyWeatherDataTest, TestSetDataEMPTY) { // Test data set empty

    EXPECT_EQ(emptyData.getTopic(), "");
    EXPECT_EQ(emptyData.getTemperature(), 0);
    EXPECT_FLOAT_EQ(emptyData.getPressure(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.getHumidity(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.getRain(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.getWind(), 0.0);

    const std::string json = mock.getJson();
    EXPECT_FALSE(emptyData.validateData(json));
}

TEST_F(WeatherDataTest, TestIsJsonFunction) { // Data in JSON format
    EXPECT_TRUE(data.validateData(mock.getJson()));
}

TEST_F(WeatherDataTest, TestDataNOTInJSONFormat) { // Test fail when not in JSON
    const std::string notJson = "This is not JSON";
    EXPECT_FALSE(data.validateData(notJson));
}

TEST_F(WeatherDataTest, TestReadData) { // Test data can be read from /device/responses
    const string returned = data.receiveData();
    const bool result = data.validateData(returned);
    EXPECT_TRUE(result);
}

TEST_F(EmptyWeatherDataTest, TestReadDataFAIL) {   // Test read data FAIL
    const string returned = emptyData.receiveData();
    const bool result = emptyData.validateData(returned);
    EXPECT_FALSE(result);
}

// ========================================================================================
// BROKER TESTS
// ========================================================================================

class BrokerTest : public testing::Test {
protected:
    MQTT_Client *client = nullptr;

    void SetUp() override {
        std::cout << "\n=== MQTT Integration Test ===" << std::endl;
        client = new MQTT_Client("test-client");

        ASSERT_TRUE(client->connect())
            << "Failed to connect to MQTT Broker";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    static json createSensorReading(
        const std::string& device_id = "device1",
        int temp = 72,
        float pressure = 1013.2,
        float humidity = 45.5,
        float rain = 0.1,
        float wind = 5.4
        ) {
        return json{
            {"device", device_id},
            {"temp", temp},
            {"pressure", pressure},
            {"humidity", humidity},
            {"rain", rain},
            {"wind", wind},
            {"timestamp", time(nullptr)}
        };
    }



    void TearDown() override {
        if (client) {
            client->disconnect();
            delete client;
            client = nullptr;
        }
        std::cout << "=== Test Complete ===\n" << std::endl;
    }
};


// ========================================================================================
// TEST CONNECTION
// ========================================================================================

TEST_F(BrokerTest, TestConnect) {
    client->connect();
    EXPECT_TRUE(client->isConnected());
}

TEST_F(BrokerTest, TestConnectFail) {
    client->disconnect();
    EXPECT_FALSE(client->isConnected());
}

TEST_F(BrokerTest, TestDisconnect) {
    client->disconnect();
    EXPECT_TRUE(client->disconnect());
}

TEST_F(BrokerTest, TestDisconnectFail) {
    client->disconnect();
    EXPECT_FALSE(client->disconnect());
}

// ========================================================================================
// TEST MESSAGE TRANSACTIONS
// ========================================================================================

TEST_F(BrokerTest, TestSubscribeToTopic) {
    const std::string topic = "test/device1/responses";

    bool result = client->subscribe(topic);
    EXPECT_TRUE(result);
}

TEST_F(BrokerTest, TestSubscribeToTopicFAIL) {
    const std::string topic = "";

    bool result = client->subscribe(topic);
    EXPECT_FALSE(result);
}

TEST_F(BrokerTest, TestPublishMessage) {
    const std::string topic = "test/device1/requests";
    const std::string payload ="Request POLL";

    bool result = client->publish(topic, payload);
    EXPECT_TRUE(result);
}

TEST_F(BrokerTest, TestPublishMessageFAIL) {
    const std::string topic = "test/device1/requests";
    const std::string payload ="";

    bool result = client->publish(topic, payload);
    EXPECT_FALSE(result);
}

TEST_F(BrokerTest, TestReceiveResponse) {
    const std::string topic = "test/device1/responses";
    ASSERT_TRUE(client->subscribe(topic));

    json sensor_data = BrokerTest::createSensorReading("device1", 72, 1013.2,
                                                45.5, 0.1, 5.4);

    ASSERT_TRUE(client->publish(topic, sensor_data)); // Simulate sensor readings to broker

    const std::string received = client->getLastMessage();
    EXPECT_NE(received,"No data in queue!\n");
}

TEST_F(BrokerTest, TestReceiveResponseFAIL) {
    const std::string received = client->getLastMessage();
    EXPECT_EQ(received,"No data in queue!\n");
}


// =========================================================================================
// SERVER TESTS
// =========================================================================================

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

TEST_F(CrowAppTest, TestDataInJson) { // Test data retrieved from server is in json format
    req.url = "/data";
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

TEST_F(CrowAppTest, TestNotInJson) {
    req.url = "/";
    req.method = "GET"_method;

    app.handle_full(req, res);

    EXPECT_EQ(res.code, 200);

    const auto json_response = crow::json::load(res.body);
    ASSERT_FALSE(json_response);
}

// ======================================================================================
// DEATH TESTS
// ======================================================================================

// TODO: Death Tests


// ======================================================================================
// DATABASE TESTS
// ======================================================================================

TEST(DatabaseTest, TestDatabaseConnection) {
    GTEST_SKIP() << "Not implemented..." << std::endl;
}

TEST(DatabaseTest, TestDatabaseConnectionFail) {
    GTEST_SKIP() << "Not implemented..." << std::endl;
}

TEST(DatabaseTest, TestDatabaseCommit) {
    GTEST_SKIP() << "Not implemented..." << std::endl;
}

TEST(DatabaseTest, TestDatabaseCommitFail) {
    GTEST_SKIP() << "Not implemented..." << std::endl;
}

TEST(DatabaseTest, TestDatabaseQuery) {
    GTEST_SKIP() << "Not implemented..." << std::endl;
}

TEST(DatabaseTest, TestDatabaseQueryFail) {
    GTEST_SKIP() << "Not implemented..." << std::endl;
}







