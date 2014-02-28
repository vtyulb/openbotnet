cmake_minimum_required(VERSION 2.8)
project(bot)

set(CMAKE_CXX_FLAGS "-Wall -Os -lpthread")
set(CMAKE_LDD_FLAGS "-static")
set(SOURCE_EXE main.cpp base64.cpp)

add_executable(bot ${SOURCE_EXE})
target_link_libraries(bot cryptopp.a)
