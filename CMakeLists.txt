# Begleit-Repository zu »openHAB & C++-Entwicklung«
#   cmake -S . -B build && cmake --build build -j2
# Jede .cpp-Datei in kapitel08 … kapitel16 ist ein eigenes Programm; die Programme aus
# mehreren Dateien (Kapitel 17 bis 20) stehen in den CMakeLists.txt ihrer Ordner.
cmake_minimum_required(VERSION 3.16)
project(OpenHABCppBuch VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra)
endif()
option(MIT_ASAN "AddressSanitizer und UndefinedBehaviorSanitizer einschalten" OFF)
if(MIT_ASAN)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# ── Bibliotheken: cpp-openhab-test-suite 2.0.0 bringt cpp-openhab-rest-client 2.0.0 mit ──
include(FetchContent)
set(OPENHAB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OPENHAB_TESTSUITE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(cpp_openhab_test_suite
  GIT_REPOSITORY https://github.com/Michdo93/cpp-openhab-test-suite.git
  GIT_TAG        9a043243e346edc1ea400366c607fdbd3215c03b)  # 2.0.0
FetchContent_MakeAvailable(cpp_openhab_test_suite)

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

# ── Einzeldateien: kapitel10_…/01_verbinden.cpp → build/kapitel10_…/01_verbinden
file(GLOB beispiele CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR}
     kapitel0[89]_*/*.cpp kapitel1[0-6]_*/*.cpp)
foreach(quelle ${beispiele})
  get_filename_component(ordner ${quelle} DIRECTORY)
  get_filename_component(name ${quelle} NAME_WE)
  set(ziel ${ordner}__${name})
  add_executable(${ziel} ${quelle})
  set_target_properties(${ziel} PROPERTIES OUTPUT_NAME ${name}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${ordner})
  target_link_libraries(${ziel} PRIVATE openhab::rest_client_cpp Threads::Threads
                        CURL::libcurl)
endforeach()

# ── Programme aus mehreren Dateien und Tests ──────────────────────────────────
include(CTest)
add_subdirectory(kapitel17_dashboard)
add_subdirectory(kapitel18_geraet)
add_subdirectory(kapitel19_tests)
add_subdirectory(kapitel20_projekte)
