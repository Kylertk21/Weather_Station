//
// Created by kylerk on 11/6/2025.
//

#include "routes.h"
#include <filesystem>

int main() {
    auto app = create_app();
    app.port(18080).multithreaded().run();

}