cmake_minimum_required(VERSION 2.6) 
project(pigell)

add_definitions(-Wall -ansi -Wno-deprecated -std=c++0x)


if(UNIX)
	if(EXISTS "/usr/local/lib/OGRE/cmake")
 
	  set(CMAKE_MODULE_PATH "/usr/local/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
	  set(OGRE_SAMPLES_INCLUDEPATH "/usr/local/share/OGRE/samples/Common/include/") # We could just *assume* that developers uses this basepath : /usr/local
 
	elseif(EXISTS "/usr/lib/OGRE/cmake")
 
	  set(CMAKE_MODULE_PATH "/usr/lib/OGRE/cmake/;${CMAKE_MODULE_PATH}")
	  set(OGRE_SAMPLES_INCLUDEPATH "/usr/share/OGRE/samples/Common/include/") # Otherwise, this one

	elseif(EXISTS "/usr/share/OGRE/cmake/modules")
 
	  set(CMAKE_MODULE_PATH "/usr/share/OGRE/cmake/modules/;${CMAKE_MODULE_PATH}")
	  set(OGRE_SAMPLES_INCLUDEPATH "/usr/share/OGRE/samples/Common/include/") # Otherwise, this one
	elseif(EXISTS "/usr/lib64/OGRE/cmake")
 
	  set(CMAKE_MODULE_PATH "/usr/lib64/OGRE/cmake/;${CMAKE_MODULE_PATH}")
	  set(OGRE_SAMPLES_INCLUDEPATH "/usr/share/OGRE/samples/Common/include/") # if on 64 bits
 
	else ()
	  message(SEND_ERROR "Failed to find module path.")
	endif(EXISTS "/usr/local/lib/OGRE/cmake")
endif(UNIX)

if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()
 
set(CMAKE_DEBUG_POSTFIX "_d")
 
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dist")
 
# Find Ogre
find_package(OGRE REQUIRED)
if(NOT "${OGRE_VERSION_NAME}" STREQUAL "Cthugha")
  #message(SEND_ERROR "You need Ogre 1.7 Cthugha to build this.")
endif()

# Find OIS
find_package(OIS REQUIRED)
if(NOT OIS_FOUND)
	message(SEND_ERROR "Failed to find OIS.")
endif()
 
# Find Boost
if (NOT OGRE_BUILD_PLATFORM_IPHONE)
	if (WIN32 OR APPLE)
		set(Boost_USE_STATIC_LIBS TRUE)
	else ()
		# Statically linking boost to a dynamic Ogre build doesn't work on Linux 64bit
		set(Boost_USE_STATIC_LIBS ${OGRE_STATIC})
	endif ()
	if (MINGW)
		# this is probably a bug in CMake: the boost find module tries to look for
		# boost libraries with name libboost_*, but CMake already prefixes library
		# search names with "lib". This is the workaround.
		set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} "")
	endif ()
	set(Boost_ADDITIONAL_VERSIONS "1.44" "1.44.0" "1.42" "1.42.0" "1.41.0" "1.41" "1.40.0" "1.40" "1.39.0" "1.39" "1.38.0" "1.38" "1.37.0" "1.37" )
	# Components that need linking (NB does not include header-only components like bind)
	set(OGRE_BOOST_COMPONENTS thread date_time)
	find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
	if (NOT Boost_FOUND)
		# Try again with the other type of libs
		set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
		find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
	endif()
	find_package(Boost QUIET)
 
	# Set up referencing of Boost
	include_directories(${Boost_INCLUDE_DIR})
	add_definitions(-DBOOST_ALL_NO_LIB)
	set(OGRE_LIBRARIES ${OGRE_LIBRARIES} ${Boost_LIBRARIES})
endif() 

# Find glog
find_library(GLOG_LIBRARIES glog)
find_path(GLOG_INCLUDE_DIRS logging.h /usr/local/include/glog)
#find_path(GLOG_INCLUDE_DIRS logging.h /usr/include/glog)
if(NOT GLOG_INCLUDE_DIRS)
	message(SEND_ERROR "Failed to find GLOG.")
endif()

# Find MyGUI
find_library(MYGUI_LIBRARIES MyGUIEngine)
find_library(MYGUI2_LIBRARIES MyGUI.OgrePlatform)
set(OGRE_LIBRARIES ${OGRE_LIBRARIES} ${MYGUI_LIBRARIES})
set(OGRE_LIBRARIES ${OGRE_LIBRARIES} ${MYGUI2_LIBRARIES})
find_path(MYGUI_INCLUDE_DIRS MyGUI.h /usr/local/include/MYGUI)
#find_path(MYGUI_INCLUDE_DIRS MyGUI.h /usr/include/MYGUI)
if(NOT MYGUI_INCLUDE_DIRS)
	message(SEND_ERROR "Failed to find MYGUI.")
endif()
set(OGRE_INCLUDE_DIRS ${OGRE_INCLUDE_DIRS} ${MYGUI_INCLUDE_DIRS})

#find Lua
find_package(Lua51 REQUIRED) 
set(Luaudio_INCLUDE_DIRS ${Luaudio_SOURCE_DIR} ${LUA_INCLUDE_DIR})


set(HDRS
	./src/game.h
	./src/options.h
	./src/cubeMap.h
	./src/eventManager.h
	./src/voxel.h
	./src/colouredVoxel.h
	./src/matterVoxel.h
	./src/worldMapState.h
	./src/tools.h
	./src/graphics/graphicsOgre.h
	./src/graphics/worldMapScene.h
	./src/graphics/worldMapGui.h
	./src/graphics/camera.h
	./src/input/inputOIS.h
)
 
set(SRCS
	./src/main.cpp
	./src/game.cpp
	./src/options.cpp
	./src/eventManager.cpp
	./src/voxel.cpp
	./src/colouredVoxel.cpp
	./src/matterVoxel.cpp
	./src/worldMapState.cpp
	./src/graphics/graphicsOgre.cpp
	./src/graphics/worldMapScene.cpp
	./src/graphics/worldMapGui.cpp
	./src/graphics/camera.cpp
	./src/input/inputOIS.cpp
)
 
include_directories( ${OIS_INCLUDE_DIRS}
	${OGRE_INCLUDE_DIRS}
	${OGRE_SAMPLES_INCLUDEPATH}
	${GLOG_INCLUDE_DIRS}
	${Luaudio_INCLUDE_DIRS}
)
 
add_executable(pigell WIN32 ${HDRS} ${SRCS})
 
set_target_properties(pigell PROPERTIES DEBUG_POSTFIX _d)
 
target_link_libraries(pigell ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${GLOG_LIBRARIES} ${LUA_LIBRARIES})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/bin)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/media)

if(MINGW OR UNIX)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/dist/bin)
endif(MINGW OR UNIX)

if(UNIX)
 
	install(TARGETS pigell
		RUNTIME DESTINATION bin
		CONFIGURATIONS All)
 
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/dist/media
		DESTINATION ./
		CONFIGURATIONS Release RelWithDebInfo Debug
	)
 
	install(FILES ${CMAKE_SOURCE_DIR}/dist/bin/plugins.cfg
		${CMAKE_SOURCE_DIR}/dist/bin/resources.cfg
		DESTINATION bin
		CONFIGURATIONS Release RelWithDebInfo Debug
	)
 
endif(UNIX)
