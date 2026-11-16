# Kapitel 17: ohdash – Dashboard nach Model-View-Controller mit Qt 6
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if(NOT Qt6_FOUND)
  message(STATUS "Qt 6 fehlt – Kapitel 17 wird übersprungen (apt install qt6-base-dev)")
  return()
endif()
set(CMAKE_AUTOMOC ON)  # moc erzeugt den Code für Q_OBJECT, Signale und Slots

# Model und Controller brauchen nur QtCore – auch die Tests in Kapitel 19 nutzen sie
add_library(ohdash_kern STATIC model.cpp controller.cpp)
target_link_libraries(ohdash_kern PUBLIC openhab::rest_client_cpp Qt6::Core
                      Threads::Threads)
target_include_directories(ohdash_kern PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ohdash main.cpp ansicht_text.cpp ansicht_qt.cpp)
target_link_libraries(ohdash PRIVATE ohdash_kern Qt6::Widgets)
set_target_properties(ohdash PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel17_dashboard)
