cmake_minimum_required(VERSION 3.10)
project(VortexVaultCpp)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Source files
set(SOURCES
    src/main.cpp
    src/KDTreeVectorDB.cpp
    src/embed.cpp
)

add_executable(vortex_vault ${SOURCES})

# Link libraries
target_link_libraries(vortex_vault pthread)
