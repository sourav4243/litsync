cmake_minimum_required(VERSION 3.10)

# Project Name and Version
project(litsync VERSION 1.0 LANGUAGES CXX)

# Require C++17 for <filesystem>
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Threading library (needed for background syncing)
find_package(Threads REQUIRED)

# Add the include directory so we can easily #include our headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Gather all soruce files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link threading to the executable
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)