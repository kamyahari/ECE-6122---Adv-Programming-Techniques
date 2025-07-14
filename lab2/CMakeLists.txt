# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(Lab2 VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/Source2.cpp)

find_package(OpenMP REQUIRED)

# Add the executable
add_executable(Lab2 ${SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/../SFML/include)

link_directories(${PROJECT_SOURCE_DIR}/../SFML/lib)

# Link the executable to the libraries in the lib directory
target_link_libraries(Lab2 PUBLIC sfml-graphics sfml-system sfml-window)

set_target_properties(
    Lab2 PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${COMMON_OUTPUT_DIR}/bin"
)

# Link OpenMP libraries
if(OpenMP_CXX_FOUND)
    target_link_libraries(Lab2 PUBLIC OpenMP::OpenMP_CXX)
endif()
