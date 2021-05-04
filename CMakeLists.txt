cmake_minimum_required(VERSION 3.7.2 FATAL_ERROR)
project(emoji-picker VERSION 0.7.0 LANGUAGES CXX)

cmake_policy(SET CMP0074 NEW)

add_definitions(
  -DPROJECT_ORGANIZATION="gazatu.xyz"
  -DPROJECT_NAME="${PROJECT_NAME}"
  -DPROJECT_VERSION="${PROJECT_VERSION}"
  # -DUSE_UINPUT=true
  # -DUSE_XDO=true
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

if (UNIX AND NOT APPLE) # LINUX (X11)
  find_library(XDOTOOL_LIB xdo REQUIRED)
  add_library(XDOTOOL STATIC SHARED IMPORTED)
  set_target_properties(XDOTOOL PROPERTIES
    IMPORTED_LOCATION ${XDOTOOL_LIB}
  )
endif ()

include_directories(${PROJECT_SOURCE_DIR}/src)

set(COMMON_SRC_FILES
  src/main.cpp
  src/main.qrc
  src/emojis.cpp
  src/emojis.qrc
  src/EmojiLineEdit.cpp
  src/EmojiLabel.cpp
  src/EmojiPicker.cpp
  src/EmojiPickerSettings.cpp
  src/EmojiTranslator.cpp
)

if (UNIX AND NOT APPLE) # LINUX
  set(PLATFORM_SRC_FILES
  )
elseif (WIN32)
  set(PLATFORM_SRC_FILES
  )
endif ()

set(SRC_FILES
  ${COMMON_SRC_FILES}
  ${PLATFORM_SRC_FILES}
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  WIN32_EXECUTABLE ON # add WinMain(...) if windows
)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  ICU::uc
)

if (UNIX AND NOT APPLE) # LINUX (X11)
  target_link_libraries(${PROJECT_NAME}
    XDOTOOL
  )
endif ()

set(STACK_SIZE_DEC 4194304) # 4MB
set(STACK_SIZE_HEX 0x400000) # 4MB

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang") # clang++ / clang-cl
  if (WIN32) # lld-link
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS /STACK:${STACK_SIZE_DEC})
  elseif (APPLE) # apple ld
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS -Wl,-stack_size,${STACK_SIZE_HEX})
  endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # g++ / mingw g++
  if (WIN32) # mingw ld
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS -Wl,--stack,${STACK_SIZE_DEC})
  elseif (APPLE) # apple ld
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS -Wl,-stack_size,${STACK_SIZE_HEX})
  endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # msvc
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS /STACK:${STACK_SIZE_DEC})
endif()
