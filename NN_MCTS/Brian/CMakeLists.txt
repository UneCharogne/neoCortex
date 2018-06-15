cmake_minimum_required( VERSION 2.6 )
project( brian )

#add the headers directory
include_directories(inc)

#add the sources
file(GLOB SOURCES "src/*.cpp")

add_library( brian ${SOURCES})
