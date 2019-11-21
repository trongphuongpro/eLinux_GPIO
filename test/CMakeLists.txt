cmake_minimum_required(VERSION 3.4)

find_package(Threads REQUIRED)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER arm-g++)
set(CMAKE_C_COMPILER arm-gcc)


set(TARGET testbench)
set(SOURCE ../src/gpio.cpp test.cpp)
add_executable(${TARGET} ${SOURCE})
target_link_libraries(${TARGET} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${TARGET} PUBLIC ../include)
target_compile_options(${TARGET} PUBLIC -std=gnu++11
										-Wall
										-Werror
										-O2
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)