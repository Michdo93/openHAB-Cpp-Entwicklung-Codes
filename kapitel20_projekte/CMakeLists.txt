# Kapitel 20: Projekte – ohctl; der ROS-2-Knoten in ros2/ wird mit colcon gebaut
add_executable(ohctl ohctl.cpp)
target_link_libraries(ohctl PRIVATE openhab::rest_client_cpp)
set_target_properties(ohctl PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel20_projekte)
install(TARGETS ohctl RUNTIME DESTINATION bin)
