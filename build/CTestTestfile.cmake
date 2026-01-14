# CMake generated Testfile for 
# Source directory: F:/CodeBlocks-AnxietyMonitor
# Build directory: F:/CodeBlocks-AnxietyMonitor/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(UnitTests "F:/CodeBlocks-AnxietyMonitor/build/Debug/AnxietyMonitorTests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "F:/CodeBlocks-AnxietyMonitor/CMakeLists.txt;193;add_test;F:/CodeBlocks-AnxietyMonitor/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(UnitTests "F:/CodeBlocks-AnxietyMonitor/build/Release/AnxietyMonitorTests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "F:/CodeBlocks-AnxietyMonitor/CMakeLists.txt;193;add_test;F:/CodeBlocks-AnxietyMonitor/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(UnitTests "F:/CodeBlocks-AnxietyMonitor/build/MinSizeRel/AnxietyMonitorTests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "F:/CodeBlocks-AnxietyMonitor/CMakeLists.txt;193;add_test;F:/CodeBlocks-AnxietyMonitor/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(UnitTests "F:/CodeBlocks-AnxietyMonitor/build/RelWithDebInfo/AnxietyMonitorTests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "F:/CodeBlocks-AnxietyMonitor/CMakeLists.txt;193;add_test;F:/CodeBlocks-AnxietyMonitor/CMakeLists.txt;0;")
else()
  add_test(UnitTests NOT_AVAILABLE)
endif()
