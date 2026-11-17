cmake_minimum_required(VERSION 3.20)
project(order_book_engine VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(OB_SANITIZE "Compila com AddressSanitizer e UBSan" OFF)
if(OB_SANITIZE AND NOT MSVC)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()
if(NOT MSVC)
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(orderbook src/order_book.cpp)
target_include_directories(orderbook PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(obctl tools/obctl.cpp)
target_link_libraries(obctl PRIVATE orderbook)

add_executable(bench_orderbook bench/bench_orderbook.cpp)
target_link_libraries(bench_orderbook PRIVATE orderbook)

enable_testing()
add_executable(orderbook_test tests/order_book_test.cpp)
target_link_libraries(orderbook_test PRIVATE orderbook)
add_test(NAME orderbook_unit COMMAND orderbook_test)
add_test(NAME obctl_replay
         COMMAND ${CMAKE_COMMAND} -DOBCTL=$<TARGET_FILE:obctl>
                 -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/examples/sessao.csv
                 -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/examples/sessao.expected
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden.cmake)
