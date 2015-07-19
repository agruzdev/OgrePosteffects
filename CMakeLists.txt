cmake_minimum_required(VERSION 2.6)

project(OgrePosteffects)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    #disable warnings appearing from OGRE
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") #dll-interface
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4512") #couldn't generate assignment operator
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127") #conditional expression is constant
    
endif()

#OGRE
set(OGRE_ROOT "$ENV{OGRE_HOME}" CACHE FILEPATH "Path to ogre root folder")
set(OGRE_BUILD "${OGRE_ROOT}/build" CACHE FILEPATH "Path to ogre build folder")
set(OGRE_INCLUDE_DIR "${OGRE_ROOT}/OgreMain/include" CACHE FILEPATH "Path to ogre includes")
set(OGRE_LIBS_DIR "${OGRE_BUILD}/lib" CACHE FILEPATH "Path to odre libs")
set(OGRE_BIN_DIR "${OGRE_BUILD}/bin" CACHE FILEPATH "Path to odre dlls")
set(OGRE_BIN_DIR_REL "${OGRE_BIN_DIR}/release")
set(OGRE_BIN_DIR_DBG "${OGRE_BIN_DIR}/debug")

include_directories(${OGRE_INCLUDE_DIR})
include_directories("${OGRE_BUILD}/include")
include_directories("${OGRE_ROOT}/Samples/Common/include")
include_directories("${OGRE_ROOT}/Components/Overlay/include")
include_directories("${OGRE_ROOT}/Components/Volume/include")
include_directories("${OGRE_ROOT}/Components/RTShaderSystem/include")

#Dependencies
set(DEPENDS_ROOT "${OGRE_ROOT}/Dependencies")
include_directories("${DEPENDS_ROOT}/include")
include_directories("${DEPENDS_ROOT}/include/OIS")

# Options
set(PostEffects_ENABLE_TEST_EFFECTS OFF CACHE BOOL "Include test post effects")

# Find Boost
set(Boost_USE_STATIC_LIBS TRUE)
set(Boost_ADDITIONAL_VERSIONS "1.44" "1.44.0" "1.42" "1.42.0" "1.41.0" "1.41" "1.40.0" "1.40" "1.39.0" "1.39" "1.38.0" "1.38" "1.37.0" "1.37" )
set(BOOST_LIBRARYDIR $ENV{BOOST_ROOT}/lib)

# Components that need linking (NB does not include header-only components like bind)
set(OGRE_BOOST_COMPONENTS thread date_time system atomic chrono)
find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} REQUIRED)
if (NOT Boost_FOUND)
    # Try again with the other type of libs
    set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
    find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} REQUIRED)
endif()

# Set up referencing of Boost
include_directories(${Boost_INCLUDE_DIR})
add_definitions(-DBOOST_ALL_NO_LIB)

# Find Loki
set(LOKI_INLCUDES "$ENV{LOKI_ROOT}/include")
if(EXISTS "${LOKI_INLCUDES}/loki/LokiExport.h")
    message(STATUS "Found LOKI: $ENV{LOKI_ROOT}")
    include_directories(${LOKI_INLCUDES})
else()
    message(FATAL_ERROR "Could not find Loki")
endif()

# Grab sources

file(GLOB_RECURSE all_sources src/*.cpp src/*.h)

foreach(f ${all_sources})
    # Get the path of the file relative to ${CMAKE_HOME_DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH SRCGR ${CMAKE_HOME_DIRECTORY} ${f})
    set(SRCGR "${SRCGR}")

    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

# defines
add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
 
if(PostEffects_ENABLE_TEST_EFFECTS)
    add_definitions(-DTEST_EFFECTS)
endif()
 
# create project
add_executable(OgrePosteffects WIN32 ${all_sources})

# include libs
target_link_libraries(OgrePosteffects ${Boost_LIBRARIES})

target_link_libraries(OgrePosteffects debug ${DEPENDS_ROOT}/lib/OIS_d.lib)
target_link_libraries(OgrePosteffects optimized  ${DEPENDS_ROOT}/lib/OIS.lib)
 
target_link_libraries(OgrePosteffects debug ${OGRE_LIBS_DIR}/Debug/OgreMain_d.lib)
target_link_libraries(OgrePosteffects debug ${OGRE_LIBS_DIR}/Debug/OgreOverlay_d.lib)

target_link_libraries(OgrePosteffects optimized ${OGRE_LIBS_DIR}/Release/OgreMain.lib)
target_link_libraries(OgrePosteffects optimized ${OGRE_LIBS_DIR}/Release/OgreOverlay.lib)

# Install project
if(WIN32)

	install(FILES ${OGRE_BIN_DIR_REL}/OgreMain.dll
		${OGRE_BIN_DIR_REL}/RenderSystem_GL.dll
        ${OGRE_BIN_DIR_REL}/RenderSystem_Direct3D9.dll
        ${OGRE_BIN_DIR_REL}/OgreOverlay.dll
		${OGRE_BIN_DIR_REL}/OIS.dll
		DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/"
		CONFIGURATIONS Release 
	)
 
	install(FILES ${OGRE_BIN_DIR_DBG}/OgreMain_d.dll
		${OGRE_BIN_DIR_DBG}/RenderSystem_GL_d.dll
        ${OGRE_BIN_DIR_DBG}/RenderSystem_Direct3D9_d.dll
        ${OGRE_BIN_DIR_DBG}/OgreOverlay_d.dll
		${OGRE_BIN_DIR_DBG}/OIS_d.dll
		DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/"
		CONFIGURATIONS Debug
	)
endif()