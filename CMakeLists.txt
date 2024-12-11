cmake_minimum_required(VERSION 3.10)
project(ISUDcutsSCIP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your source files
set(SOURCES
    filename.cpp
)

# Specify the path to the SCIP installation
# You need to adjust these paths according to your SCIP installation directory
set(SCIP_DIR "C:/Program Files/SCIPOptSuite 9.1.0")

# Include directories
include_directories(${SCIP_DIR}/include)

# Library directories
link_directories(${SCIP_DIR}/lib)

# Add executable
add_executable(ISUDcutsSCIP ${SOURCES})

# Link SCIP libraries
target_link_libraries(ISUDcutsSCIP
    libscip
    libsoplex
    zimpl
)
