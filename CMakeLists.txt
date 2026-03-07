cmake_minimum_required(VERSION 3.25)

project(Cobalt)

set(CMAKE_C_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

set(SOURCE_FILES src/main.c
  src/frontend/scanner.c
  src/frontend/scanner.h
  src/frontend/token.c
  src/frontend/token.h
  src/utils/list.h
  src/utils/list.c)

add_executable(cobalt ${SOURCE_FILES})
target_include_directories(cobalt PRIVATE src)
