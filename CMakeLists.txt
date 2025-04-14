cmake_minimum_required(VERSION 3.15)
project(HTTPServer)

#add_definitions(-DUNICODE -D_UNICODE)

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

# Build threader library
add_library(threader STATIC
    src/threader.cpp
)

# Build main server executable
add_executable(HTTPServer
    src/main.cpp
    src/connection_context.cpp
    src/llmserver.cpp
    src/service.cpp
    src/socket_manager.cpp
    src/tasynchndlr.cpp
    src/LLMReqProcessor.cpp
    src/tthreadpool.cpp
)

# Link libraries for main server
target_link_libraries(HTTPServer PRIVATE syncflag threader ws2_32)

# Test: syncflag
add_executable(test_syncflag
    src/test_syncflag.cpp
)
target_link_libraries(test_syncflag PRIVATE syncflag ws2_32)

# Test: threader
add_executable(test_threader
    src/test_threader.cpp
)
target_link_libraries(test_threader PRIVATE syncflag threader ws2_32)
