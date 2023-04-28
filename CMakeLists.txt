cmake_minimum_required(VERSION 3.0.0)

project(Logger VERSION 0.1.1)

set(CMAKE_CXX_STANDARD 17)

message(BEGIN BUILD)

file(GLOB SRC ./*cpp ./*hpp)

if (NOT "${SRC}" STREQUAL "")
    message(src successful)
endif ()

file(GLOB DIR ./*h)

if (NOT "${DIR}" STREQUAL "")
    message(dir successful)
endif ()

add_library(${PROJECT_NAME} ${SRC})

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC ${DIR}
)
target_link_libraries(
        ${PROJECT_NAME}
        pthread
)
option(ABLE_TEST "enable_test" TRUE)

if (ABLE_TEST)
    message(ENABLE_TEST)
    add_subdirectory(test1)
endif ()

message(END BUILD)
