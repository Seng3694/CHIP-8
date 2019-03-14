cmake_minimum_required(VERSION 2.4)

project(CHIP-8 C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	set(CONFIGURATION "x64")
else() 
	set(CONFIGURATION "x86")
endif() 

set(HEADERS
	include/config.h
	include/emulator.h
	include/registers.h
)

set(SOURCES
	src/emulator.c
	src/main.c
)

include_directories(include)
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

find_package(SDL2 REQUIRED)

if(SDL2_FOUND)
	include_directories( ${SDL2_INCLUDE_DIR} )
	target_link_libraries( ${PROJECT_NAME} ${SDL2_LIBRARY} )
endif()

add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND	${CMAKE_COMMAND} -E copy_if_different 
			${SDL2_PATH}/lib/${CONFIGURATION}/SDL2.dll
			${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND	${CMAKE_COMMAND} -E copy_directory
			${CMAKE_SOURCE_DIR}/roms
			${CMAKE_CURRENT_BINARY_DIR}/roms
)
