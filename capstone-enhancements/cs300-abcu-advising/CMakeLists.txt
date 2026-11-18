cmake_minimum_required(VERSION 3.16)
project(abcu_advising LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  # The benchmark is only meaningful with optimization on.
  set(CMAKE_BUILD_TYPE Release)
endif()

# Everything except the entry points, shared by the app, the tests and the benchmark.
add_library(abcu_core STATIC
  src/course.cpp
  src/course_store.cpp
  src/catalog_loader.cpp
  src/prerequisite_graph.cpp
  src/advising_app.cpp
  third_party/csvparser/CSVparser.cpp)
target_include_directories(abcu_core PUBLIC src third_party/csvparser)

add_executable(abcu_advising src/main.cpp)
target_link_libraries(abcu_advising PRIVATE abcu_core)

add_executable(abcu_tests tests/test_main.cpp)
target_link_libraries(abcu_tests PRIVATE abcu_core)
target_compile_definitions(abcu_tests PRIVATE
  ABCU_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/ABCU_Advising_Program_Input.csv")

add_executable(abcu_benchmark benchmark/benchmark.cpp)
target_link_libraries(abcu_benchmark PRIVATE abcu_core)
target_compile_definitions(abcu_benchmark PRIVATE
  ABCU_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/ABCU_Advising_Program_Input.csv")

enable_testing()
add_test(NAME abcu_tests COMMAND abcu_tests)
