
cmake_minimum_required(VERSION 3.21)

project(GDChess VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Platform configuration
if(WIN32)
    message(STATUS "Configuring GDChess for Windows")

elseif(ANDROID)
    message(STATUS "Configuring GDChess for Android")

else()
    message(FATAL_ERROR "Unsupported platform. Windows and Android only.")
endif()

# Collect source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create mod library
add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
)

# Locate Geode SDK
if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR
        "Unable to find Geode SDK! Please define GEODE_SDK."
    )
endif()

message(STATUS "Found Geode SDK: $ENV{GEODE_SDK}")

# Include Geode SDK
add_subdirectory(
    "$ENV{GEODE_SDK}"
    "${CMAKE_CURRENT_BINARY_DIR}/geode"
)

# Configure Geode mod
setup_geode_mod(${PROJECT_NAME})

