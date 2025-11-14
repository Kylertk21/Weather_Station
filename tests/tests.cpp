//
// Created by kylerk on 11/10/2025.
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include "../src/weather_data.h"
#include "../src/routes.h"
#include <vector>

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
    Weather_Data data = Weather_Data();
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
    Weather_Data emptyData = Weather_Data();
    MockJsonProvider mock;

    void SetUp() override {
        emptyData.set_data(
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

    EXPECT_EQ(data.get_topic(), "device1/responses");
    EXPECT_EQ(data.get_temperature(), 72);
    EXPECT_FLOAT_EQ(data.get_pressure(), 1013.2);
    EXPECT_FLOAT_EQ(data.get_humidity(), 45.5);
    EXPECT_FLOAT_EQ(data.get_rain(), 0.1);
    EXPECT_FLOAT_EQ(data.get_wind(), 5.4);

    const std::string json = mock.getJson();
    EXPECT_TRUE(data.validateData(json));

}

TEST_F(EmptyWeatherDataTest, TestSetDataEMPTY) { // Test data set empty

    EXPECT_EQ(emptyData.get_topic(), "");
    EXPECT_EQ(emptyData.get_temperature(), 0);
    EXPECT_FLOAT_EQ(emptyData.get_pressure(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.get_humidity(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.get_rain(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.get_wind(), 0.0);

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
    const string returned = data.readData();
    const bool result = data.validateData(returned);
    EXPECT_TRUE(result);
}

TEST_F(EmptyWeatherDataTest, TestReadDataFAIL) {   // Test read data FAIL
    const string returned = emptyData.readData();
    const bool result = emptyData.validateData(returned);
    EXPECT_FALSE(result);
}

// ========================================================================================
// INTEGRATION TESTS
// ========================================================================================

class MQTT_Test_Client {
private:
    struct mosquitto *client = nullptr;
    std::string broker_host;
    int broker_port = 0;
    std::atomic<bool> connected{false};
    std::atomic<bool> message_received{false};
    std::vector<std::string> received_messages;
    std::string client_id;

    static void on_connect_callback(struct mosquitto *mosq, void *obj, int rc) {
        auto *test_client = static_cast<MQTT_Test_Client*>(obj);
        test_client->connected = (rc == 0);

        if (rc == 0) {
            std::cout << "[MQTT] Connected Successfully To Broker" << std::endl;
        } else {
            std::cerr << "[MQTT] Connection Failed!: " << rc << std::endl;
        }
    }
public:
    MQTT_Test_Client(const std::string& id = "test-client") : client_id(id) {
        const char* host_env = std::getenv("MQTT_BROKER_HOST");
        broker_host = host_env ? host_env : "mqtt-broker";

        const char* port_env = std::getenv("MQTT_BROKER_PORT");
        broker_port = port_env ? std::stoi(port_env) : 1883;

        std::cout << "[MQTT] Configured for " << broker_host << ":" << broker_port << std::endl;
    }

};


TEST(BrokerTest, TestSendData) { // Test data can be sent to /device/requests
    GTEST_SKIP() << "Not implemented...";
}

TEST(BrokerTest, TestSendDataFAIL) { // Test send data FAIL
    GTEST_SKIP() << "Not implemented";
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

// ======================================================================================
// DEATH TESTS
// ======================================================================================

// TODO: Death Tests









