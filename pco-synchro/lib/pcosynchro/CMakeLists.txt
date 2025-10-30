cmake_minimum_required(VERSION 3.14)
project(pcosynchro LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the static library
add_library(pcosynchro STATIC
    ../../src/pcoconditionvariable.cpp
    ../../src/pcohoaremonitor.cpp
    ../../src/pcologger.cpp
    ../../src/pcomanager.cpp
    ../../src/pcomutex.cpp
    ../../src/pcosemaphore.cpp
    ../../src/pcothread.cpp
)

# Include directories
target_include_directories(pcosynchro PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>
    $<INSTALL_INTERFACE:include/pcosynchro>
)

# Installation rules
install(TARGETS pcosynchro
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ../../src/
    DESTINATION include/pcosynchro
    FILES_MATCHING PATTERN "*.h"
)

