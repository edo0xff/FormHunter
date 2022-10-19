cmake_minimum_required(VERSION 2.8.12)
project(CppFormHunter CXX)

set (CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(cfhunter src/Main.cpp src/HttpRecorder.cpp src/Metric.cpp)

target_link_libraries(cfhunter ${CONAN_LIBS})

conan_target_link_libraries(cfhunter)
