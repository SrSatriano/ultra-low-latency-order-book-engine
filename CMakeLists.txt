cmake_minimum_required(VERSION 3.20)
project(order_book_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_BENCHMARKS "Build benchmark targets" OFF)

add_subdirectory(src/core)

add_executable(orderbook_test tests/order_book_test.cpp)
target_link_libraries(orderbook_test PRIVATE orderbook_core)
target_include_directories(orderbook_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(ENABLE_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
