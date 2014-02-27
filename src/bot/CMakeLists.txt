cmake_minimum_required(VERSION 2.8)
project(bot)

set(SOURCE_EXE main.cpp base64.cpp)
add_executable(bot ${SOURCE_EXE})
target_link_libraries(main cryptopp.a)
