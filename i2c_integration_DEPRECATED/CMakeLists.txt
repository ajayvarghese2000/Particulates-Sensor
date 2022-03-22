cmake_minimum_required(VERSION 3.0.0)
project(i2c_integration VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(i2c_integration main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
