# Kapitel 18: ohgeraet – der Raspberry Pi (oder jeder Linux-Rechner) als Gerät für openHAB
option(MIT_GPIOD "GPIO über libgpiod 2 (C++-Anbindung) ansteuern (Raspberry Pi)" OFF)

add_library(ohgeraet_kern STATIC messwerte.cpp ausgang.cpp geraet.cpp)
target_include_directories(ohgeraet_kern PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ohgeraet_kern PUBLIC openhab::rest_client_cpp)
if(MIT_GPIOD)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiodcxx>=2)
  target_compile_definitions(ohgeraet_kern PRIVATE MIT_GPIOD)
  target_link_libraries(ohgeraet_kern PUBLIC PkgConfig::GPIOD)
endif()

add_executable(ohgeraet main.cpp)
target_link_libraries(ohgeraet PRIVATE ohgeraet_kern)
set_target_properties(ohgeraet PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel18_geraet)
install(TARGETS ohgeraet RUNTIME DESTINATION bin)
