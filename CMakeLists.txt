cmake_minimum_required(VERSION 3.10)
project(LocalSearchProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
set(SOURCES
    src/lab7.cpp
    src/DistanceMatrix.cpp
    src/BaseSolver.cpp
    src/LocalSearchSolver.cpp
    src/LSNLocalSearchSolver.cpp
    src/Solution.cpp
    src/RandomSolution.cpp
    src/Utils.cpp
)

# Add executable
add_executable(LocalSearchExecutable ${SOURCES})

# Include directories
target_include_directories(LocalSearchExecutable PRIVATE src/)

# Add optimization flags
target_compile_options(LocalSearchExecutable PRIVATE
    -O3            # Optimize for maximum performance
    -march=native  # Use native processor features for optimization
    -flto          # Link-time optimization
    -funroll-loops # Unroll loops for performance
)

# Add custom command to copy the data folder
add_custom_command(
    TARGET LocalSearchExecutable POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/data
            ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying instances folder to build directory"
)
