cmake_minimum_required(VERSION 3.10)
project(VortexVaultCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio
    ${Boost_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/KDTreeVectorDB.cpp
    src/embed.cpp
)

add_executable(vortex_vault ${SOURCES})

# Compiler flags for optimization
target_compile_options(vortex_vault PRIVATE -Wall -Wextra -O2)

# Link libraries
target_link_libraries(vortex_vault pthread)
