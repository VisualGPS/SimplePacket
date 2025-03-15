cmake_minimum_required(VERSION 3.20)

#
# About the project
#
project(TestPacket)

if(MSVC)
	# Disable MSVC warnings that suggest making code non-portable.  
	set(CMAKE_CXX_FLAGS "/wd4996 ${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE) 
else()
	set(CMAKE_CXX_FLAGS " -Wall")
	set (CMAKE_CXX_STANDARD 17)
endif ()

#
# Add subdirectories
#
#add_subdirectory(none)

#
# Include directories
#
include_directories(
    ../SimplePacketLib
)

#
# Add executable name and project files
#
add_executable(TestPacket
	main.cpp 
)

#
# Add additional libraries
#
target_link_libraries(TestPacket SimplePacketLib)


