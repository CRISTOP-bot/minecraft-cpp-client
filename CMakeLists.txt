cmake_minimum_required(VERSION 3.20)
project(minecraft_cpp_client VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mc_nbt src/nbt/reader.cpp)
target_include_directories(mc_nbt PUBLIC include)
target_compile_options(mc_nbt PRIVATE
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wsign-conversion>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->"
)

include(CTest)
if(BUILD_TESTING)
    add_executable(nbt_tests tests/nbt_tests.cpp)
    target_link_libraries(nbt_tests PRIVATE mc_nbt)
    add_test(NAME nbt_tests COMMAND nbt_tests)
endif()
