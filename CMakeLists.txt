cmake_minimum_required(VERSION 3.16)
project(AnxietyMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ============================================================================
# Find Dependencies
# ============================================================================

# wxWidgets
find_package(wxWidgets 3.0 REQUIRED COMPONENTS core base adv)
include(${wxWidgets_USE_FILE})

# Code::Blocks SDK - Try to find via environment variable or standard paths
if(DEFINED ENV{CODEBLOCKS_SDK_PATH})
    set(CODEBLOCKS_SDK_PATH $ENV{CODEBLOCKS_SDK_PATH})
elseif(WIN32)
    # Common Windows installation paths
    set(CODEBLOCKS_SDK_PATH "C:/Program Files/CodeBlocks" CACHE PATH "Code::Blocks installation path")
else()
    set(CODEBLOCKS_SDK_PATH "/usr/include/codeblocks" CACHE PATH "Code::Blocks SDK path")
endif()

# Include directories for Code::Blocks SDK
if(WIN32)
    set(CB_INCLUDE_DIRS
        "${CODEBLOCKS_SDK_PATH}/include"
        "${CODEBLOCKS_SDK_PATH}/sdk"
        "${CODEBLOCKS_SDK_PATH}/include/tinyxml"
    )
    set(CB_LIB_DIR "${CODEBLOCKS_SDK_PATH}/lib")
else()
    set(CB_INCLUDE_DIRS
        "${CODEBLOCKS_SDK_PATH}"
        "${CODEBLOCKS_SDK_PATH}/sdk"
        "/usr/include/codeblocks/sdk"
        "/usr/include/codeblocks/tinyxml"
    )
    set(CB_LIB_DIR "/usr/lib")
endif()

# ============================================================================
# Plugin Sources
# ============================================================================

set(PLUGIN_SOURCES
    src/AnxietyMonitor.cpp
    src/DataCollector.cpp
    src/AnxietyScorer.cpp
    src/CSVWriter.cpp
    src/EventHandlers.cpp
    src/UIComponents.cpp
)

set(PLUGIN_HEADERS
    src/AnxietyMonitor.h
    src/DataCollector.h
    src/AnxietyScorer.h
    src/CSVWriter.h
    src/EventHandlers.h
    src/UIComponents.h
    src/MetricsData.h
)

# ============================================================================
# Build Plugin as Shared Library
# ============================================================================

add_library(AnxietyMonitor SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

target_include_directories(AnxietyMonitor PRIVATE
    ${CB_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(AnxietyMonitor PRIVATE
    ${wxWidgets_LIBRARIES}
)

# Link Code::Blocks SDK library on Windows
if(WIN32)
    target_link_libraries(AnxietyMonitor PRIVATE
        "${CB_LIB_DIR}/codeblocks.lib"
    )
endif()

# Set output properties
set_target_properties(AnxietyMonitor PROPERTIES
    PREFIX ""
    OUTPUT_NAME "AnxietyMonitor"
)

# Platform-specific suffix
if(WIN32)
    set_target_properties(AnxietyMonitor PROPERTIES SUFFIX ".dll")
else()
    set_target_properties(AnxietyMonitor PROPERTIES SUFFIX ".so")
endif()

# ============================================================================
# Installation
# ============================================================================

if(WIN32)
    set(CB_PLUGINS_DIR "C:/Program Files/CodeBlocks/share/codeblocks/plugins" 
        CACHE PATH "Code::Blocks plugins directory")
else()
    set(CB_PLUGINS_DIR "/usr/share/codeblocks/plugins"
        CACHE PATH "Code::Blocks plugins directory")
endif()

install(TARGETS AnxietyMonitor
    LIBRARY DESTINATION ${CB_PLUGINS_DIR}
    RUNTIME DESTINATION ${CB_PLUGINS_DIR}
)

install(FILES resources/anxiety_monitor.zip
    DESTINATION ${CB_PLUGINS_DIR}
    OPTIONAL
)

# ============================================================================
# Custom Commands
# ============================================================================

# Create resources zip for Code::Blocks
add_custom_command(TARGET AnxietyMonitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "AnxietyMonitor plugin built successfully!"
    COMMAND ${CMAKE_COMMAND} -E echo "Copy to: ${CB_PLUGINS_DIR}"
)

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "=== AnxietyMonitor Plugin Configuration ===")
message(STATUS "Code::Blocks SDK Path: ${CODEBLOCKS_SDK_PATH}")
message(STATUS "wxWidgets Version: ${wxWidgets_VERSION_STRING}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install Path: ${CB_PLUGINS_DIR}")
message(STATUS "============================================")
message(STATUS "")
