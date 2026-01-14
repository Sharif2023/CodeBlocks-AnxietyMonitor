cmake_minimum_required(VERSION 3.16)
project(AnxietyMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# CRITICAL: Build Configuration for Code::Blocks 25.03 Bundled Compiler
# ----------------------------------------------------------------------------

# 1. Compiler (Bundled with Code::Blocks)
set(CMAKE_CXX_COMPILER "E:/Program Files/CodeBlocks/MinGW/bin/g++.exe")

# 2. Main Include Paths
#    - Code::Blocks SDK Source
#    - wxWidgets Headers (3.2.4)
include_directories(
    "E:/Program Files/codeblocks_25.03/src/include"
    "E:/Program Files/codeblocks_25.03/src/sdk/wxscintilla/include"
    "E:/Program Files/codeblocks_25.03/src/base/tinyxml"
    "E:/Program Files/codeblocks_25.03/src/include/tinyxml"
    "E:/Program Files/wxWidgets-3.2.4/include"
)

# 3. Setup.h Location (Config header)
#    We try to use the one from wxWidgets-3.2.4 MSVC lib folder as fallback for defines
#    MinGW specific one would be better but if missing, this usually works for headers
include_directories("E:/Program Files/wxWidgets-3.2.4/lib/vc14x_x64_dll/mswu")

# 4. Definitions
add_compile_definitions(
    CB_PRECOMP
    WXUSINGDLL
    __WXMSW__
    _UNICODE
    UNICODE
    CODEBLOCKS_SDK_INCLUDED
    HAVE_W32API_H
)

# ----------------------------------------------------------------------------
# Sources
# ----------------------------------------------------------------------------
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h")

add_library(AnxietyMonitor SHARED ${SOURCES} ${HEADERS})

# ----------------------------------------------------------------------------
# Linking (The Secret Sauce)
# ----------------------------------------------------------------------------
# We link directly against the DLLs in Code::Blocks installation directory
# This ensures 100% ABI compatibility with the running IDE
# Link against Code::Blocks and wxWidgets DLLs directly (Absolute paths to handle spaces)
# link_directories("E:/Program Files/CodeBlocks") # Avoid this, spaces in path break ld

target_link_libraries(AnxietyMonitor PRIVATE 
    "E:/Program Files/CodeBlocks/codeblocks.dll"
    "E:/Program Files/CodeBlocks/wxmsw32u_gcc_custom.dll"
)

# ----------------------------------------------------------------------------
# Final Output
# ----------------------------------------------------------------------------
set_target_properties(AnxietyMonitor PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
)

# Install target for packaging
install(TARGETS AnxietyMonitor RUNTIME DESTINATION bin)
