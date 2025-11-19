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
        std::cout << "\n=== Weather Data Test Setup ===" << std::endl;
        chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(time_point);
        string formatted_time = WeatherData::convertTime(tt);

        data.setData(
            1,
            "device1/responses",
            72,
            1013.2,
            45.5,
            0.1,
            5.4,
            formatted_time
            );
        ON_CALL(mock, getJson())
            .WillByDefault(testing::Return(
                R"({
                        "topic":"device1/responses",
                        "temp":72,
                        "pressure":1013.2,
                        "humidity":45.5,
                        "rain":0.1,
                        "wind":5.4,
                        "timestamp":{"2025-11-18 15:45:05"}
                                    })"));
    }
    void TearDown() override {
        std::cout << "=== Weather Data Test Complete ===\n" << std::endl;
    }

};

class EmptyWeatherDataTest : public testing::Test {
protected:
    WeatherData emptyData = WeatherData();
    MockJsonProvider mock;

    void SetUp() override {
        emptyData.setData(
            1,
            "",
            0,
            0.0,
            0.0,
            0.0,
            0.0,
            "" );
        ON_CALL(mock, getJson())
            .WillByDefault(testing::Return(
                R"({})"));
    }
};

TEST_F(WeatherDataTest, TestSetData) { // Test data is set in class structure
    EXPECT_EQ(data.getDataID(), 1);
    EXPECT_EQ(data.getTopic(), "device1/responses");
    EXPECT_EQ(data.getTemperature(), 72);
    EXPECT_FLOAT_EQ(data.getPressure(), 1013.2);
    EXPECT_FLOAT_EQ(data.getHumidity(), 45.5);
    EXPECT_FLOAT_EQ(data.getRain(), 0.1);
    EXPECT_FLOAT_EQ(data.getWind(), 5.4);
    EXPECT_NE(data.getTimeStamp(), time(nullptr));
    EXPECT_GT(data.getTimeStamp(), 0);

    const std::string json = mock.getJson();
    EXPECT_TRUE(data.validateData(json));

}

TEST_F(EmptyWeatherDataTest, TestSetDataEMPTY) { // Test data set empty
    EXPECT_EQ(emptyData.getDataID(), 0);
    EXPECT_EQ(emptyData.getTopic(), "");
    EXPECT_EQ(emptyData.getTemperature(), 0);
    EXPECT_FLOAT_EQ(emptyData.getPressure(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.getHumidity(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.getRain(), 0.0);
    EXPECT_FLOAT_EQ(emptyData.getWind(), 0.0);
    EXPECT_EQ(emptyData.getTimeStamp(), time(nullptr));
    EXPECT_EQ(emptyData.getTimeStamp(), time(nullptr));

    const std::string json = mock.getJson();
    EXPECT_FALSE(emptyData.validateData(json));
}

TEST_F(WeatherDataTest, TestTimeStampPopulated) {
    time_t timestamp = data.getTimeStamp();
    EXPECT_GT(timestamp, 0);

    time_t now = time(nullptr);
    EXPECT_NEAR(timestamp, now, 5);
}

TEST_F(WeatherDataTest, TestValidateTimeStampFormat) {
    time_t ts = data.getTimeStamp();
    std::string formatted = WeatherData::convertTime(ts);

    std::tm tm{};
    localtime_r(&ts, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    EXPECT_EQ(formatted, oss.str());
}

TEST_F(WeatherDataTest, TestInvalidTimeStampFormat) {
    std::string bad_ts = "not-a-timestamp";

    std::tm tm{};
    std::istringstream ss(bad_ts);

    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    EXPECT_TRUE(ss.fail());
}

TEST_F(WeatherDataTest, TestIsJsonFunction) {
    EXPECT_TRUE(data.validateData(mock.getJson()));
}

// ========================================================================
// SERVER - MQTT REQUEST AND RESPONSE
// ========================================================================

TEST_F(WeatherDataTest, TestRequestData) {
    MQTT_Client client("test");
    const std::string request = "request update";
    ASSERT_TRUE(WeatherData::connectBroker());
    ASSERT_TRUE(WeatherData::requestData(request));

    std::string receivedData = data.receiveData();
    EXPECT_EQ(data.validateData(receivedData), true);
}

TEST_F(WeatherDataTest, TestRequestDataFAIL) {
    const std::string request = "request update";
    ASSERT_TRUE(WeatherData::connectBroker());
    ASSERT_TRUE(WeatherData::requestData(request));

    std::string receivedData = data.receiveData();
    EXPECT_NE(data.validateData(receivedData), true);
}


TEST_F(WeatherDataTest, TestReceiveData) { // Test data can be read from /device/responses
    const string returned = data.receiveData();
    const bool result = data.validateData(returned);
    EXPECT_TRUE(result);
}

TEST_F(EmptyWeatherDataTest, TestReceiveDataFAIL) {   // Test read data FAIL
    const string returned = emptyData.receiveData();
    const bool result = emptyData.validateData(returned);
    EXPECT_FALSE(result);
}


// ======================================================================================
// WEATHER DATA DEATH TESTS
// ======================================================================================

TEST_F(WeatherDataTest, DeathInvalidTopic) {
    EXPECT_DEATH(
    data.setData(1, "", 72, 1013.2, 45.5, 0.1, 5.4, "2025-11-18 15:45:05"),
    ".*");
}

TEST_F(WeatherDataTest, DeathInvalidTemperature) {
    EXPECT_DEATH(
    data.setData(1, "device1", -9999, 1013.2, 45.5, 0.1, 5.4, "2025-11-18 15:45:05"),
    ".*");
}

TEST_F(WeatherDataTest, DeathDataNOTInJSONFormat) { // Test fail when not in JSON
    const std::string notJson = "This is not JSON";
    EXPECT_DEATH(data.validateData(notJson), ".*");
}


// ========================================================================================
// BROKER TESTS
// ========================================================================================

class BrokerTest : public testing::Test {
protected:
    MQTT_Client *client = nullptr;

    void SetUp() override {
        std::cout << "\n=== MQTT Integration Test Setup ===" << std::endl;
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
        float wind = 5.4,
        time_t timestamp = time(nullptr)
        ) {
        return json{
            {"device", device_id},
            {"temp", temp},
            {"pressure", pressure},
            {"humidity", humidity},
            {"rain", rain},
            {"wind", wind},
            {"timestamp", timestamp}
        };
    }



    void TearDown() override {
        if (client) {
            client->disconnect();
            delete client;
            client = nullptr;
        }
        std::cout << "=== MQTT Integration Test Complete ===\n" << std::endl;
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
                                                45.5, 0.1, 5.4,time(nullptr));

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

    void SetUp() override {
        std::cout << "\n=== Crow App Test Setup ===" << std::endl;
    }

    static json createSensorReading(
    const std::string& device_id = "device1",
    int temp = 72,
    float pressure = 1013.2,
    float humidity = 45.5,
    float rain = 0.1,
    float wind = 5.4,
    time_t timestamp = time(nullptr)
    ) {
        return json{
                    {"device", device_id},
                    {"temp", temp},
                    {"pressure", pressure},
                    {"humidity", humidity},
                    {"rain", rain},
                    {"wind", wind},
                    {"timestamp", timestamp
                    }
        };
   }

    void TearDown() override {
        std::cout << "=== Crow App Test Complete ===\n" << std::endl;
    }

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

TEST_F(CrowAppTest, TestIndexRouteWrongMethod) { // Index page not available
    req.url = "/";
    req.method = "POST"_method;

    app.handle_full(req, res);

    EXPECT_NE(res.code, 200);
    EXPECT_TRUE(res.code == 404 || res.code == 405);
}

TEST_F(CrowAppTest, TestBadRoute) {
    req.url = "/doesntexist";
    req.method = "GET"_method;

    app.handle_full(req, res);

    EXPECT_EQ(res.code, 404);
}

TEST_F(CrowAppTest, TestPostData) {
    req.url = "/data/weather";
    json postData = createSensorReading();
    std::string body = postData.dump();

    req.body = body;
    req.add_header("Content-Type", "application/json");
    app.handle_full(req, res);

    EXPECT_EQ(res.code, 200);
    EXPECT_TRUE(res.body.find("success") != std::string::npos);
}

TEST_F(CrowAppTest, TestGetDataInJson) { // Test data retrieved from server is in json format
    req.url = "/data/weather";
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
    EXPECT_NE(json_response["timestamp"].d(), time(nullptr));
    EXPECT_GT(json_response["timestamp"].d(), 0);
}

TEST_F(CrowAppTest, TestGetNotInJson) {
    req.url = "/";
    req.method = "GET"_method;

    app.handle_full(req, res);

    ASSERT_EQ(res.code, 200);

    const auto json_response = crow::json::load(res.body);
    EXPECT_FALSE(json_response);
}

TEST_F(CrowAppTest, TestGetDataMissingParams) {
    req.url = "/data/weather?device=";
    req.method = "GET"_method;
    ASSERT_EQ(res.code, 200);

    app.handle_full(req, res);

    EXPECT_EQ(res.code, 400);
}

// ======================================================================================
// DATABASE TESTS
// ======================================================================================

class DataBaseTest : public testing::Test {
protected:
    WeatherDataBase* db = nullptr;
    std::string test_device = "test-device1";

    void SetUp() override {
        std::string host = std::getenv("DATABASE_HOST") // if specified use DATABASE_HOST
            ? std::getenv("DATABASE_HOST")
            : "localhost"; // otherwise use localhost
        int port = std::getenv("DATABASE_PORT")
            ? std::stoi(std::getenv("DATABASE_PORT"))
            : 5432;
        std::cout << "\n Database Test Setup ===" << std::endl;
        std::cout << "[DB] Connecting to " << host << ":" << port << std::endl;

        db = new WeatherDataBase();
    }
    void TearDown() override {
        if (db) {
            if (db->isConnected()) {
                db->clearAllReadings();
                db->disconnect();
            }
            delete db;
            db = nullptr;
        }
        std::cout << "=== Database Test Complete ===\n" << std::endl;
    }

// ======================================================================================
// DATABASE CONNECTION TESTS
// ======================================================================================

};

TEST_F(DataBaseTest, TestDatabaseConnection) {
    bool connected = db->connect();

    ASSERT_TRUE(connected);
    EXPECT_TRUE(db->isConnected());
}

TEST_F(DataBaseTest, TestDatabaseConnectionFail) {
    WeatherDataBase bad_db("invalid_host", 5555, "wrong_db", "wrong_user", "wrong_pass");

    bool connected = bad_db.connect();

    EXPECT_FALSE(connected);
    EXPECT_FALSE(bad_db.isConnected());
}

TEST_F(DataBaseTest, TestReconnect) {
    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());

    db->disconnect();
    EXPECT_FALSE(db->isConnected());

    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());
}

TEST_F(DataBaseTest, TestMultipleConnections) {
    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());
    // attempt second connect
    EXPECT_TRUE(db->isConnected());
}

// ======================================================================================
// DATABASE TRANSACTION TESTS
// ======================================================================================

TEST_F(DataBaseTest, TestDatabaseCommit) {
    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());

    WeatherData data;
    data.setData(1, "test-topic", 72, 1013.2, 45.5, 0.1, 5.4, "2025-11-18 15:45:05");

    const bool inserted = db->commitReading(data);
    ASSERT_TRUE(inserted) << "Failed to insert reading";

    const int count = db->getDataCount("test-topic");
    EXPECT_GT(count, 0);
}

TEST_F(DataBaseTest, TestDatabaseCommitMultiple) {
    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());

    for (int i = 1; i <= 5; i++) {
        WeatherData data;
        data.setData(1, "test-topic", 72, 1013.2, 45.5, 0.1, 5.4, "2025-11-18 15:45:05");
        ASSERT_TRUE(db->commitReading(data)) << "Failed to insert reading " << i;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    int count = db->getDataCount("test-topic");
    EXPECT_EQ(count, 5);
}

TEST_F(DataBaseTest, TestDatabaseFailNotConnected) {
    const auto now = std::chrono::system_clock::now();
    const std::time_t timer = std::chrono::system_clock::to_time_t(now);

    WeatherData data;

    data.setData(1, "test-topic", 72, 1013.2, 45.5, 0.1, 5.4, "2025-11-18 15:45:05");

    bool inserted = db->commitReading(data);

    EXPECT_FALSE(inserted) << "Should fail when not connected";
}

TEST_F(DataBaseTest, TestDatabaseCommitFailEmptyCommit) {
    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());

    WeatherData emptyData;
    emptyData.setData(0, "", 0, 0.0, 0.0, 0.0, 0.0, "2025-11-18 15:45:05");

    const bool inserted = db->commitReading(emptyData);
    ASSERT_FALSE(inserted);

    const int count = db->getDataCount(0);
    EXPECT_EQ(count, 0);
}

TEST_F(DataBaseTest, TestDatabaseCommitFailInvalidID) {
    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());

    WeatherData data;
    data.setData(0, "test-topic", 72, 1013.2, 45.5, 0.1, 5.4, "2025-11-18 15:45:05");

    const bool inserted = db->commitReading(data);
    ASSERT_FALSE(inserted);

    const int count = db->getDataCount(0);
    EXPECT_EQ(count, 0);
}
// TODO: test removing from database

// ======================================================================================
// DATABASE QUERY TESTS
// ======================================================================================

TEST_F(DataBaseTest, TestDatabaseQuery) {
    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());

    WeatherData data;
    data.setData(1, "test-topic", 72, 1013.2, 45.5, 0.1, 5.4, "2025-11-18 15:45:05");
    ASSERT_TRUE(db->commitReading(data)); // TODO: finish TestDatabaseQuery

    string timestamp = data.convertTime(*"2025-11-18 15:45:05");
    WeatherData queriedData = db->queryReadingByID(1);

    EXPECT_EQ(queriedData.getDataID(), 1);
    EXPECT_EQ(queriedData.getTopic(), "test-topic");
    EXPECT_EQ(queriedData.getTemperature(), 72);
    EXPECT_EQ(queriedData.getPressure(), 1013.2);
    EXPECT_EQ(queriedData.getHumidity(), 45.5);
    EXPECT_EQ(queriedData.getRain(), 0.1);
    EXPECT_EQ(queriedData.getWind(), 5.4);
    EXPECT_EQ(queriedData.getTimeStamp(), timestamp);

}

TEST_F(DataBaseTest, TestDatabaseQueryByTopic) {
    ASSERT_TRUE(db->connect());
    EXPECT_TRUE(db->isConnected());

    for (int i = 1; i <= 3; i++) {
        WeatherData data;
        data.setData(i, "test-topic", 70 + 1, 1013.0, 15.0, 0.1, 5.0,
                "2025-11-18 15:45:05" + std::to_string(i));
        ASSERT_TRUE(db->commitReading(data));
    }

    std::vector<WeatherData> results = db->queryReadingByID(1);
}

TEST_F(DataBaseTest, TestDatabaseQueryFail) {
    GTEST_SKIP() << "Not implemented..." << std::endl;
}







