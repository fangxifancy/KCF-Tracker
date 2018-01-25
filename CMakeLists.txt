cmake_minimum_required(VERSION 2.8)
set(CMAKE_PREFIX_PATH "~/SDCard/opencv-3.3.0/share/OpenCV")
project(KCF_track)

find_package(OpenCV REQUIRED)

if(NOT WIN32)
ADD_DEFINITIONS("-std=c++0x -O3")
endif(NOT WIN32)

include_directories(src) 
FILE(GLOB_RECURSE sourcefiles "src/*.cpp")
add_executable( KCF ${sourcefiles} )
target_link_libraries( KCF ${OpenCV_LIBS})




