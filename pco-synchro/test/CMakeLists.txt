cmake_minimum_required(VERSION 3.14)
project(pcosynchrotest LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the executable
add_executable(pcosynchrotest
    ../src/pcomanager.cpp
    ../src/pcothread.cpp
    ../src/pcomutex.cpp
    ../src/pcosemaphore.cpp
    ../src/pcoconditionvariable.cpp
    main.cpp
)

# Include directories
target_include_directories(pcosynchrotest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# Link libraries
target_link_libraries(pcosynchrotest
    pthread
    gtest
)

