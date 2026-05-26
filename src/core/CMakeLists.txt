add_library(orderbook_core
  order_book.cpp
)
target_include_directories(orderbook_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
