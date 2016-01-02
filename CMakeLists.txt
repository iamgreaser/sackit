cmake_minimum_required(VERSION 2.8.4)
project(sackit)

set(SACKIT_SOURCES
	effects.c
	fixedmath.c
	mixer.c
	objects.c
	playroutine.c
	playroutine_effects.c
	playroutine_nna.c
	tables.c
)

set(SACKIT_HEADERS
	sackit_internal.h
	sackit.h
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include_directories(${CMAKE_SOURCE_DIR})
add_library(sackit ${SACKIT_SOURCES} ${SACKIT_HEADERS})

option(BUILD_SACKIT_COMPARE "Build sackit compare" ON)
option(BUILD_SACKIT_PLAY "Build sackit play" ON)
option(BUILD_SACKIT_EZJACK "Build sackit ezjack" ON)
option(BUILD_SACKIT_SFX "Build sackit sfx" ON)
option(BUILD_SACKIT_CONVERT "Build sackit convert" ON)

if (BUILD_SACKIT_COMPARE OR BUILD_SACKIT_PLAY OR BUILD_SACKIT_EZJACK OR BUILD_SACKIT_SFX)
	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIR})

	if (BUILD_SACKIT_EZJACK)
		find_package(Jack REQUIRED)
		include_directories(${JACK_INCLUDE_DIRS})
	endif ()
endif ()

if (BUILD_SACKIT_COMPARE)
	add_executable(sackit_compare app_compare.c)
	target_link_libraries(sackit_compare ${SDL2_LIBRARY} sackit)
endif ()

if (BUILD_SACKIT_PLAY)
	add_executable(sackit_play app_play.c)
	target_link_libraries(sackit_play ${SDL2_LIBRARY} sackit)
endif ()

if (BUILD_SACKIT_EZJACK)
	add_executable(sackit_ezjack app_ezjack.c ezjack.c)
	target_link_libraries(sackit_ezjack ${SDL2_LIBRARY} ${JACK_LIBRARIES} sackit)
endif ()

if (BUILD_SACKIT_SFX)
	add_executable(sackit_sfx app_sfx.c)
	target_link_libraries(sackit_sfx ${SDL2_LIBRARY} sackit)
endif ()

if (BUILD_SACKIT_CONVERT)
	add_executable(sackit_convert app_convert.c)
	target_link_libraries(sackit_convert sackit)
endif ()

