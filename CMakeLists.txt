cmake_minimum_required(VERSION 2.6.2)
project(SkeletonApp)

SET(USERSPECIFICPATH /mnt/dscvr/apps)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${USERSPECIFICPATH}/FionaLib)
INCLUDE_DIRECTORIES(${USERSPECIFICPATH}/FionaLib/include)

add_definitions(-DLINUX_BUILD)

ADD_EXECUTABLE(SkeletonApp main.cpp)
TARGET_LINK_LIBRARIES(SkeletonApp GL GLU GLEW glut ${USERSPECIFICPATH}/FionaLib/libFionaLib.a ${USERSPECIFICPATH}/FionaLib/linux/lib/libvrpn.a pthread)
