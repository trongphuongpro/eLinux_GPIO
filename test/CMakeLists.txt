cmake_minimum_required(VERSION 3.4)

find_package(Threads REQUIRED)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER arm-g++)
set(CMAKE_C_COMPILER arm-gcc)

add_compile_options(
	-std=gnu++11
	-Wall
	-Werror
	-O2
)


include_directories(../include)
set(SOURCE ../src/gpio.cpp test.cpp)
add_executable(testbench ${SOURCE})
target_link_libraries(testbench ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS testbench RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)