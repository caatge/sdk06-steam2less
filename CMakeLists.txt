cmake_minimum_required(VERSION 3.14)

project(
  fixes
  LANGUAGES CXX C)


add_compile_definitions(ZYDIS_STATIC_BUILD) 

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(fixes SHARED src/main.cpp src/Zydis.c src/safetyhook.cpp)

set_target_properties(fixes PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(fixes PROPERTIES OUTPUT_NAME "fixes_i486")
include_directories(fixes src/)

#target_link_libraries(fixes PUBLIC engine)
