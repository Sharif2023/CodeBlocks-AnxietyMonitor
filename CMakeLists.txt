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

# FORCE MinGW wxWidgets configuration if using MinGW and libs exist
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND WIN32)
    set(MINGW_WX_PATH "E:/wxWidgets/install")
    set(MINGW_WX_LIB_PATH "${MINGW_WX_PATH}/lib/gcc_x64_dll")
    if(EXISTS "${MINGW_WX_LIB_PATH}/libwxbase33u.a")
        message(STATUS "Using forced MinGW wxWidgets 3.3 at: ${MINGW_WX_PATH}")
        set(wxWidgets_FOUND TRUE)
        set(wxWidgets_INCLUDE_DIRS 
            "${MINGW_WX_PATH}/include"
            "${MINGW_WX_LIB_PATH}/mswu"
        )
        set(wxWidgets_LIBRARIES
            "${MINGW_WX_LIB_PATH}/libwxbase33u.a"
            "${MINGW_WX_LIB_PATH}/libwxmsw33u_core.a"
            "${MINGW_WX_LIB_PATH}/libwxmsw33u_adv.a"
        )
        set(wxWidgets_DEFINITIONS -DWXUSINGDLL -D__WXMSW__ -D_UNICODE)
    endif()
endif()

if(USE_SYSTEM_WXWIDGETS AND NOT wxWidgets_FOUND)
    # Try to find wxWidgets
    find_package(wxWidgets 3.0 QUIET COMPONENTS core base adv)
    
    if(wxWidgets_FOUND)
        include(${wxWidgets_USE_FILE})
        message(STATUS "Found wxWidgets: ${wxWidgets_VERSION_STRING}")
    else()
        # Try common wxWidgets installation paths on Windows
        if(WIN32)
            set(WX_SEARCH_PATHS
                "E:/Program Files/wxWidgets/install"
                "E:/Program Files/wxWidgets-3.2.4"
                "E:/Program Files/wxWidgets-3.2.4"
                "E:/Program Files/wxWidgets-3.2"
                "E:/Program Files/wxWidgets"
                "E:/Program Files/wxWidgets-3.2.4"
                "E:/Program Files/wxWidgets"
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
                    else()
                        # Manual fallback for MinGW pre-built binaries
                        set(WX_LIB_PATH_GCC "${WX_PATH}/lib/gcc_x64_dll")
                        set(WX_LIB_PATH_MSVC "${WX_PATH}/lib/vc14x_x64_dll")
                        
                        if(EXISTS "${WX_LIB_PATH_GCC}")
                            message(STATUS "Manually configuring MinGW wxWidgets at: ${WX_PATH}")
                            set(wxWidgets_FOUND TRUE)
                            set(wxWidgets_INCLUDE_DIRS 
                                "${WX_PATH}/include"
                                "${WX_LIB_PATH_GCC}/mswu"
                            )
                            # MinGW uses .a libraries
                            set(wxWidgets_LIBRARIES
                                "${WX_LIB_PATH_GCC}/libwxbase32u.a"
                                "${WX_LIB_PATH_GCC}/libwxmsw32u_core.a"
                                "${WX_LIB_PATH_GCC}/libwxmsw32u_adv.a"
                            )
                            set(wxWidgets_DEFINITIONS "-DWXUSINGDLL" "-D__WXMSW__" "-D_UNICODE")
                            break()
                        elseif(EXISTS "${WX_LIB_PATH_MSVC}")
                            message(STATUS "Manually configuring MSVC wxWidgets at: ${WX_PATH}")
                            set(wxWidgets_FOUND TRUE)
                            set(wxWidgets_INCLUDE_DIRS 
                                "${WX_PATH}/include"
                                "${WX_LIB_PATH_MSVC}/mswu"
                            )
                            set(wxWidgets_LIBRARIES
                                "${WX_LIB_PATH_MSVC}/wxbase32u.lib"
                                "${WX_LIB_PATH_MSVC}/wxmsw32u_core.lib"
                                "${WX_LIB_PATH_MSVC}/wxmsw32u_adv.lib"
                            )
                            set(wxWidgets_DEFINITIONS "-DWXUSINGDLL" "-D__WXMSW__" "-D_UNICODE")
                            break()
                        endif()
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
# Build Plugin as Shared Library (only when wxWidgets is available)
# ============================================================================
if(NOT STANDALONE_BUILD)
    add_library(AnxietyMonitor SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

    # Include directories
    target_include_directories(AnxietyMonitor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

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
    
    # NOTE: CODEBLOCKS_SDK_INCLUDED requires full CB compilation
    # Using stub classes instead for simpler deployment
    # target_compile_definitions(AnxietyMonitor PRIVATE CODEBLOCKS_SDK_INCLUDED)
    
    # Link wxWidgets
    if(wxWidgets_FOUND)
        target_include_directories(AnxietyMonitor PRIVATE ${wxWidgets_INCLUDE_DIRS})
        target_compile_definitions(AnxietyMonitor PRIVATE ${wxWidgets_DEFINITIONS})
        target_link_libraries(AnxietyMonitor PRIVATE ${wxWidgets_LIBRARIES})
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

    # Installation
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
endif()

# ============================================================================
# Build Unit Tests (always available - no wxWidgets needed)
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
