# Variables
set(nlohmann_json_INCLUDE_DIRS "../json-develop/include")
set(nlohmann_json_LIBRARIES "../json-develop/include")

# Targets
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_INCLUDE_DIRS})
target_link_libraries(nlohmann_json INTERFACE ${nlohmann_json_LIBRARIES})