cmake_minimum_required(VERSION 3.16)
project(AnxietyMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ============================================================================
# Configuration Options
# ============================================================================
option(USE_SYSTEM_WXWIDGETS "Use system wxWidgets (set to OFF for standalone build)" ON)
option(STANDALONE_BUILD "Build without Code::Blocks SDK dependencies" OFF)

# ============================================================================
# Code::Blocks SDK Path
# ============================================================================
if(DEFINED ENV{CODEBLOCKS_SDK_PATH})
    set(CODEBLOCKS_SDK_PATH $ENV{CODEBLOCKS_SDK_PATH})
elseif(WIN32)
    set(CODEBLOCKS_SDK_PATH "C:/Program Files/CodeBlocks" CACHE PATH "Code::Blocks installation path")
else()
    set(CODEBLOCKS_SDK_PATH "/usr/include/codeblocks" CACHE PATH "Code::Blocks SDK path")
endif()

# ============================================================================
# Find wxWidgets (optional - can build standalone without it)
# ============================================================================
set(wxWidgets_FOUND FALSE)

if(USE_SYSTEM_WXWIDGETS)
    # Try to find wxWidgets
    find_package(wxWidgets 3.0 QUIET COMPONENTS core base adv)
    
    if(wxWidgets_FOUND)
        include(${wxWidgets_USE_FILE})
        message(STATUS "Found wxWidgets: ${wxWidgets_VERSION_STRING}")
    else()
        # Try common wxWidgets installation paths on Windows
        if(WIN32)
            set(WX_SEARCH_PATHS
                "C:/wxWidgets-3.2.4"
                "C:/wxWidgets-3.2"
                "C:/wxWidgets"
                "E:/wxWidgets-3.2.4"
                "E:/wxWidgets"
                "$ENV{WXWIN}"
            )
            
            foreach(WX_PATH ${WX_SEARCH_PATHS})
                if(EXISTS "${WX_PATH}/include/wx/wx.h")
                    set(wxWidgets_ROOT_DIR "${WX_PATH}")
                    find_package(wxWidgets 3.0 QUIET COMPONENTS core base adv)
                    if(wxWidgets_FOUND)
                        include(${wxWidgets_USE_FILE})
                        message(STATUS "Found wxWidgets at: ${WX_PATH}")
                        break()
                    endif()
                endif()
            endforeach()
        endif()
    endif()
endif()

# ============================================================================
# Standalone Build Mode (without wxWidgets)
# ============================================================================
if(NOT wxWidgets_FOUND)
    message(WARNING "wxWidgets not found - building in STANDALONE mode")
    message(WARNING "The plugin will compile but requires wxWidgets for full functionality")
    set(STANDALONE_BUILD ON)
    add_definitions(-DSTANDALONE_BUILD)
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

# Include directories
target_include_directories(AnxietyMonitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(NOT STANDALONE_BUILD)
    # Include Code::Blocks SDK directories
    if(WIN32)
        target_include_directories(AnxietyMonitor PRIVATE
            "${CODEBLOCKS_SDK_PATH}/include"
            "${CODEBLOCKS_SDK_PATH}/sdk"
        )
    else()
        target_include_directories(AnxietyMonitor PRIVATE
            "${CODEBLOCKS_SDK_PATH}"
            "${CODEBLOCKS_SDK_PATH}/sdk"
            "/usr/include/codeblocks/sdk"
        )
    endif()
    
    # Link wxWidgets
    if(wxWidgets_FOUND)
        target_link_libraries(AnxietyMonitor PRIVATE ${wxWidgets_LIBRARIES})
    endif()
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
# Build Unit Tests (always available)
# ============================================================================
add_executable(AnxietyMonitorTests
    tests/unit_tests.cpp
    src/AnxietyScorer.cpp
)

target_include_directories(AnxietyMonitorTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Enable testing
enable_testing()
add_test(NAME UnitTests COMMAND AnxietyMonitorTests)

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

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "=== AnxietyMonitor Plugin Configuration ===")
message(STATUS "Code::Blocks SDK Path: ${CODEBLOCKS_SDK_PATH}")
if(wxWidgets_FOUND)
    message(STATUS "wxWidgets: Found (${wxWidgets_VERSION_STRING})")
else()
    message(STATUS "wxWidgets: NOT FOUND (standalone build)")
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Standalone Mode: ${STANDALONE_BUILD}")
message(STATUS "============================================")
message(STATUS "")

if(STANDALONE_BUILD)
    message(STATUS "")
    message(STATUS "*** STANDALONE BUILD MODE ***")
    message(STATUS "To build the full plugin with wxWidgets support:")
    message(STATUS "1. Download wxWidgets 3.2+ from https://www.wxwidgets.org/downloads/")
    message(STATUS "2. Build wxWidgets or use pre-built binaries")
    message(STATUS "3. Set wxWidgets_ROOT_DIR or WXWIN environment variable")
    message(STATUS "4. Re-run CMake")
    message(STATUS "")
    message(STATUS "You can still run unit tests: cmake --build build --target AnxietyMonitorTests")
    message(STATUS "")
endif()
