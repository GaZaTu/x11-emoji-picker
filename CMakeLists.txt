cmake_minimum_required(VERSION 3.7.2)
project(emoji-picker VERSION 0.3.0)

cmake_policy(SET CMP0074 NEW)

add_definitions(
  -DPROJECT_ORGANIZATION="gazatu.xyz"
  -DPROJECT_NAME="${PROJECT_NAME}"
  -DPROJECT_VERSION="${PROJECT_VERSION}"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# if windows
# set(Qt5_USE_STATIC_LIBS ON)
# set(Qt5_USE_STATIC_RUNTIME ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

find_package(ICU REQUIRED COMPONENTS uc)

if (UNIX AND NOT APPLE)
  find_library(XDOTOOL_LIB xdo REQUIRED)
  add_library(XDOTOOL STATIC SHARED IMPORTED)
  set_target_properties(XDOTOOL PROPERTIES
    IMPORTED_LOCATION ${XDOTOOL_LIB}
  )
endif ()

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/main.qrc
  src/emojis.cpp
  src/emojis.hpp
  src/EmojiLineEdit.cpp
  src/EmojiLineEdit.hpp
  src/EmojiLabel.cpp
  src/EmojiLabel.hpp
  src/EmojiPicker.cpp
  src/EmojiPicker.hpp
  src/EmojiPickerSettings.cpp
  src/EmojiPickerSettings.hpp
  src/EmojiTranslator.cpp
  src/EmojiTranslator.hpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  ICU::uc
)

if (UNIX AND NOT APPLE)
  target_link_libraries(${PROJECT_NAME}
    XDOTOOL
  )
endif ()
