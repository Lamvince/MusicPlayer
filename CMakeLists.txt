cmake_minimum_required(VERSION 3.21)
project(MusicPlayer)

set(CMAKE_CXX_STANDARD 14)

add_library(strmiids.lib STATIC IMPORTED GLOBAL)
set_target_properties(strmiids.lib PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/strmiids.lib)

include_directories(${DSHOW}/dshow.h)
include_directories(${STRMIIDS}/strmiids.lib)


add_executable(MusicPlayer main.cpp)
target_link_libraries(MusicPlayer strmiids.lib)