cmake_minimum_required(VERSION 3.15)
project(HTTPServer)

# Set C++17 and x86 target
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force x86 build (important for syncflag)
if (MSVC)
    set(CMAKE_GENERATOR_PLATFORM Win32)
endif()

# Include headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Build syncflag library
add_library(syncflag STATIC
    src/syncflag.cpp
)

# Build main executable
add_executable(HTTPServer
    src/main.cpp
    src/connection_context.cpp
    src/llmserver.cpp
    src/service.cpp
    src/socket_manager.cpp
)

# Link libraries
target_link_libraries(HTTPServer PRIVATE syncflag ws2_32)
