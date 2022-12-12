add_executable(test1 test.cc ../Logger.cpp ../Logger.h ../AsyncLogging.cpp ../AsyncLogging.h ../FixBuffer.hpp)
target_link_libraries(test1
    pthread
)