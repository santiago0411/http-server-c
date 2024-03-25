cmake_minimum_required(VERSION 3.5)
project(http-server-c C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Collect all source files recursively from the 'app' directory
file(GLOB_RECURSE SOURCES app/*.c)
file(GLOB_RECURSE HEADERS app/*.h)

# Include directories
include_directories(app)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
