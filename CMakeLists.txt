cmake_minimum_required(VERSION 3.10)

project(uchuva LANGUAGES CXX)

# Options
option(BUILD_TESTING "Build tests" ON)
option(ENABLE_ASAN "Enable ASAN" OFF)
option(ENABLE_UBSAN "Enable UBSAN" OFF)
option(ENABLE_TSAN "Enable TSAN" OFF)

# Pleasant IDE experience
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ standard and warnings
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Sanitizer options
if (ENABLE_ASAN)
  message(STATUS "Enabling AddressSanitizer (ASan)")
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

if (ENABLE_MSAN)
  message(STATUS "Enabling MemorySanitizer (MSan)")
  add_compile_options(-fsanitize=memory -fsanitize-memory-track-origins -fno-omit-frame-pointer)
  add_link_options(-fsanitize=memory -fsanitize-memory-track-origins -fno-omit-frame-pointer)
endif()

if (ENABLE_UBSAN)
  message(STATUS "Enabling UndefinedBehaviorSanitizer (UBSan)")
  add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined -fno-omit-frame-pointer)
endif()

if (ENABLE_TSAN)
  message(STATUS "Enabling ThreadSanitizer (TSan)")
  add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
  add_link_options(-fsanitize=thread -fno-omit-frame-pointer)
endif()

# epoll is for Linux
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
endif()

# kqueue is for macOS
if (APPLE)
endif()


# Include the threads lib for UNIX.
if (UNIX AND NOT APPLE)
endif()

add_library(uchuva
  # The guts
  src/eventloop.cc
  src/tcpclient.cc
  src/tcpserver.cc
  # The parser
  src/http-lexer.cc
  src/http-parser.cc
)

target_include_directories(uchuva
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ECHO SERVER BUILD
add_executable(echoserver tests/echo_server.cc)
target_link_libraries(echoserver PRIVATE uchuva)

add_executable(httpparser tests/parser.cc)
target_link_libraries(httpparser PRIVATE uchuva)
