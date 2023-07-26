# Specify the path to the nlohmann/json library
cmake_minimum_required(VERSION 3.12)
project(MyServer)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)

# Add the executable
add_executable(server server.cpp)

# Add the executable
add_executable(client client2.cpp)

# Link against the required libraries
# Find and import the nlohmann_json library
find_package(nlohmann_json REQUIRED HINTS "../json-develop")

# target_link_libraries(server PRIVATE jsoncpp)

# Add the nlohmann/json library to your target
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(client PRIVATE nlohmann_json::nlohmann_json)