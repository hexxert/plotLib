cmake_minimum_required(VERSION 2.8)
project(libplotlib_project)
file(GLOB_RECURSE libplotlib_SOURCES "src/*/*.c")
file(GLOB_RECURSE libplotlib_HEADERS "src/*/*.h")

set (libplotlib_INCLUDE_DIRS "")
foreach (_headerFile ${libplotlib_HEADERS})
	get_filename_component(_dir ${_headerFile} PATH)
	list(APPEND libplotlib_INCLUDE_DIRS ${_dir}) 
endforeach()
list(REMOVE_DUPLICATES libplotlib_INCLUDE_DIRS)

find_package( OpenGL REQUIRED)
find_package( GLUT REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${libplotlib_INCLUDE_DIRS})

add_library(libplotlib ${libplotlib_SOURCES})

