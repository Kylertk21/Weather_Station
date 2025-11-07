//
// Created by kylerk on 11/6/2025.
//

#include "crow.h"
#include <filesystem>

int main() {
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")([]() {
    crow::mustache::set_base(std::filesystem::path(__FILE__).parent_path().parent_path().string() + "/src/templates");
    auto page = crow::mustache::load_text("index.html");
    return page;
  });

  app.port(18080).multithreaded().run();
}