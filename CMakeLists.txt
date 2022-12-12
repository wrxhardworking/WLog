cmake_minimum_required(VERSION 3.0.0)
project(Logger VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
include(CTest)
enable_testing()

add_executable(Logger Logger.cpp AsyncLogging.cpp AsyncLogging.h FixBuffer.hpp FixBuffer.hpp FixBuffer.hpp)
add_subdirectory(test1)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
target_link_libraries(Logger
    pthread
)
include(CPack)
