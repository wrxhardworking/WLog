add_executable(test test.cc)

target_link_libraries(
        test
        ${PROJECT_NAME}
)