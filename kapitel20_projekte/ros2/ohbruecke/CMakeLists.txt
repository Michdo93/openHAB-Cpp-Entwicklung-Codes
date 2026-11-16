# Kapitel 20: ohbruecke – wird mit colcon gebaut, nicht mit dem Haupt-CMakeLists.txt
#   source /opt/ros/jazzy/setup.bash
#   colcon build --base-paths kapitel20_projekte/ros2
cmake_minimum_required(VERSION 3.16)
project(ohbruecke LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# cpp-openhab-rest-client: installiert (find_package) oder direkt von GitHub
find_package(OpenHABRestClientCpp QUIET)
if(NOT OpenHABRestClientCpp_FOUND)
  include(FetchContent)
  set(OPENHAB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(cpp_openhab_rest_client
    GIT_REPOSITORY https://github.com/Michdo93/cpp-openhab-rest-client.git
    GIT_TAG        7c0364552cf3e55068ad9d658d166401b9373918)
  FetchContent_MakeAvailable(cpp_openhab_rest_client)
endif()

add_executable(haus_bruecke src/haus_bruecke.cpp)
target_link_libraries(haus_bruecke openhab::rest_client_cpp)
ament_target_dependencies(haus_bruecke rclcpp std_msgs)

install(TARGETS haus_bruecke DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})
ament_package()
