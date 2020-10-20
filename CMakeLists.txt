set(CMAKE_C_COMPILER "gcc-6")
set(CMAKE_CXX_COMPILER "g++-6")

cmake_minimum_required(VERSION 3.0.0)
project(emoji-picker VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

find_library(ICU_UC_LIB icuuc REQUIRED)
add_library(ICU_UC SHARED IMPORTED)
set_target_properties(ICU_UC PROPERTIES
  IMPORTED_LOCATION ${ICU_UC_LIB}
)

find_library(XDOTOOL_LIB xdo REQUIRED)
add_library(XDOTOOL SHARED IMPORTED)
set_target_properties(XDOTOOL PROPERTIES
  IMPORTED_LOCATION ${XDOTOOL_LIB}
)

add_executable(emoji-picker
  src/main.cpp
  src/main.qrc
  src/EmojiLineEdit.cpp
  src/EmojiLineEdit.hpp
  src/EmojiLabel.cpp
  src/EmojiLabel.hpp
  src/EmojiPicker.cpp
  src/EmojiPicker.hpp
)

target_link_libraries(emoji-picker
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  ICU_UC
  XDOTOOL
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CPack)
