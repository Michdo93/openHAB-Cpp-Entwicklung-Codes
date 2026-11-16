# Kapitel 19: Tests mit GoogleTest und CTest – Unit-Tests ohne openHAB,
# Integrationstests mit openHAB
#   ctest --test-dir build -L unit            nur Unit-Tests
#   ctest --test-dir build -L integration     braucht OPENHAB_URL und OPENHAB_TOKEN
find_package(GTest)
if(NOT GTest_FOUND)
  message(STATUS "GoogleTest fehlt – nur vorabtest (apt install libgmock-dev)")
endif()
include(GoogleTest)

add_library(testhilfe STATIC attrappe.cpp)
target_include_directories(testhilfe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(testhilfe PUBLIC Threads::Threads)

# Ein Testprogramm aus einer Datei; die Tests darin findet gtest_discover_tests selbst
function(buch_test name label)
  if(NOT GTest_FOUND)
    return()
  endif()
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE testhilfe GTest::gmock_main ${ARGN})
  set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel19_tests)
  gtest_discover_tests(${name} PROPERTIES LABELS ${label} TIMEOUT 120)
endfunction()

buch_test(test_messwerte unit ohgeraet_kern)
buch_test(test_geraet unit ohgeraet_kern)
buch_test(test_geraet_openhab integration ohgeraet_kern openhab::test_suite_cpp)
if(TARGET ohdash_kern)  # nur mit Qt 6
  find_package(Qt6 COMPONENTS Test REQUIRED)
  buch_test(test_model unit ohdash_kern Qt6::Test)
  buch_test(test_controller unit ohdash_kern Qt6::Test)
endif()

add_executable(vorabtest vorabtest.cpp)
target_link_libraries(vorabtest PRIVATE openhab::test_suite_cpp)
set_target_properties(vorabtest PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/kapitel19_tests)
add_test(NAME vorabtest COMMAND vorabtest)
set_tests_properties(vorabtest PROPERTIES LABELS integration SKIP_RETURN_CODE 77)
