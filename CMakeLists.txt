cmake_minimum_required(VERSION 3.0.0)

project(WLog VERSION 1.1.2)

set(CMAKE_CXX_STANDARD 17)

message(BEGIN-BUILD)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)

if (NOT "${SRC}" STREQUAL "")
    message(src-loading)
endif ()

file(GLOB DIR ${PROJECT_SOURCE_DIR}/include/*.h)

if (NOT "${DIR}" STREQUAL "")
    message(dir-loading)
endif ()

add_library(${PROJECT_NAME} ${SRC})

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${DIR}
        ${PROJECT_SOURCE_DIR}
)
target_link_libraries(
        ${PROJECT_NAME}
        pthread
)

#这是设置后端buffer刷新间隔时间的全局宏
target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE FLUSH_TIME=3
)


message("if don`t want to have the test module input in your cmake file :
set (ABLE_TEST OFF CACHE BOOL 'Enable test' FORCE)")


option(ABLE_TEST "enable_test" TRUE)

if (ABLE_TEST)
    message(ENABLE-TEST)
    add_subdirectory(test1)
endif ()

message(END-BUILD)
