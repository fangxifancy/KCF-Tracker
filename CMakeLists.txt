#cmake_minimum_required(VERSION 2.8)
#project(KCF)
#
#find_package(OpenCV REQUIRED)
#
#if(NOT WIN32)
#ADD_DEFINITIONS("-std=c++0x -O3")
#endif(NOT WIN32)
#
#include_directories(src)
#FILE(GLOB_RECURSE sourcefiles "src/*.cpp")
#add_executable( KCF ${sourcefiles} )
#target_link_libraries( KCF ${OpenCV_LIBS})

SET(execName sin)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 OLD)
    cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

SET(EXECUTABLE_OUTPUT_PATH ".")

SET(VERSION_REQ_OCV "3.2")
SET(VERSION_REQ_CUDA "8.0")

IF(WIN32) # Windows
    SET(ZED_INCLUDE_DIRS $ENV{ZED_INCLUDE_DIRS})
    SET(ZED_LIBRARIES $ENV{ZED_LIBRARIES_64})
    SET(ZED_LIBRARY_DIR $ENV{ZED_LIBRARY_DIR})
    SET(OPENCV_DIR $ENV{OPENCV_DIR})
    SET(EIGEN3_INCLUDE_DIR $ENV{EIGEN_DIR})
ELSE() # Linux
    find_package(ZED REQUIRED)
    find_package(Eigen3 REQUIRED)

    ##For Jetson, OpenCV4Tegra is based on OpenCV2.4
    exec_program(uname ARGS -p OUTPUT_VARIABLE CMAKE_SYSTEM_NAME2)
    if ( CMAKE_SYSTEM_NAME2 MATCHES "aarch64" OR CMAKE_SYSTEM_NAME2 MATCHES "armv7l" )
        SET(VERSION_REQ_OCV "2.4")
        SET(VERSION_REQ_CUDA "")
    endif()
    add_definitions(-Wno-format-extra-args)

    ##For Jetson TX1
    if ( CMAKE_SYSTEM_NAME2 MATCHES "aarch64")
        unset(CUDA_USE_STATIC_CUDA_RUNTIME CACHE)
        option(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
        message ("CUDA_USE_STATIC_CUDA_RUNTIME : ${CUDA_USE_STATIC_CUDA_RUNTIME}")
        ##to prevent from opencv_dep_cudart dependencies error...
        ## cmake with -DCUDA_USE_STATIC_CUDA_RUNTIME=false can also be called.
    endif()

    SET(SPECIAL_OS_LIBS "pthread" "X11")
ENDIF(WIN32)

set(OpenCV_DIR "/home/fangxi/local/opencv3.2.0/share/OpenCV")
find_package(OpenCV ${VERSION_REQ_OCV} REQUIRED)
find_package(CUDA ${VERSION_REQ_CUDA} REQUIRED)

include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

link_directories(${ZED_LIBRARY_DIR})
link_directories(${OpenCV_LIBRARY_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

SET(SRC_FOLDER src)
FILE(GLOB_RECURSE SRC_FILES "${SRC_FOLDER}/*.cpp")

ADD_EXECUTABLE(${execName} ${SRC_FILES})
set_property(TARGET ${execName} PROPERTY OUTPUT_NAME ${execName})

add_definitions(-std=c++0x -g -O3)

TARGET_LINK_LIBRARIES(${execName}
        ${ZED_LIBRARIES}
        ${SPECIAL_OS_LIBS}
        ${OpenCV_LIBRARIES}
        ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY} ${CUDA_npp_LIBRARY}
        )
