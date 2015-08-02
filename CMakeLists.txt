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

# macro for generating recursive directories
macro(GENERATE_RECURSE_DIRS BASE_DIR OUT_LIST)
    #list(APPEND SUFFIXES "/*" "/*/*" "/*/*/*" "/*/*/*/*" "/*/*/*/*/*" "/*/*/*/*/*/*")
    list(APPEND SUFFIXES "/*" "/*/*" "/*/*/*" "/*/*/*/*")
    foreach(suffix ${SUFFIXES})
        list(APPEND ${OUT_LIST} "${BASE_DIR}${suffix}")
    endforeach()
endmacro(GENERATE_RECURSE_DIRS)

# Fond OGRE
set(OGRE_ROOT "$ENV{OGRE_HOME}" CACHE FILEPATH "Path to ogre root folder")
generate_recurse_dirs(${OGRE_ROOT} OGRE_SEARCH_DIRS)

find_path(OGRE_INCLUDE_DIR Ogre.h PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_INCLUDE_BUILD_SETTINGS OgreBuildSettings.h PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_INCLUDE_TRAYS SdkTrays.h PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_INCLUDE_OVERLAY OgreOverlay.h PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_INCLUDE_RT_SHADERSYSTEM OgreRTShaderSystem.h PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_INCLUDE_PARTICLES_SYSTEM OgreParticleFXPlugin.h PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)

find_path(OGRE_LIBS_DIR_DBG OgreMain_d.lib PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_LIBS_DIR_REL OgreMain.lib PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)

find_path(OGRE_BIN_DIR_DBG OgreMain_d.dll PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_BIN_DIR_REL OgreMain.dll PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)


include_directories(${OGRE_INCLUDE_DIR})
include_directories(${OGRE_INCLUDE_BUILD_SETTINGS})
include_directories(${OGRE_INCLUDE_TRAYS})
include_directories(${OGRE_INCLUDE_OVERLAY})
include_directories(${OGRE_INCLUDE_RT_SHADERSYSTEM})
include_directories(${OGRE_INCLUDE_PARTICLES_SYSTEM})

# Dependencies
# OIS
find_path(OGRE_INCLUDE_OIS OIS.h PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_OIS_LIB_DBG OIS_d.lib  PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_OIS_LIB_REL OIS.lib  PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_OIS_BIN_DBG OIS_d.dll  PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(OGRE_OIS_BIN_REL OIS.dll  PATHS ${OGRE_SEARCH_DIRS} NO_DEFAULT_PATH)

include_directories(${OGRE_INCLUDE_OIS})


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
generate_recurse_dirs("$ENV{LOKI_ROOT}" LOKI_FIND_DIRS)
find_path(LOKI_INLCUDES NAMES "loki/LokiExport.h" PATHS ${LOKI_FIND_DIRS} NO_DEFAULT_PATH) 
if(NOT  LOKI_INLCUDES MATCHES LOKI_INLCUDES-NOTFOUND)
    message(STATUS "Found LOKI: $ENV{LOKI_ROOT}")
    include_directories(${LOKI_INLCUDES})
else()
    message(FATAL_ERROR "Could not find Loki in $ENV{LOKI_ROOT}/include")
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

target_link_libraries(OgrePosteffects debug ${OGRE_OIS_LIB_DBG}/OIS_d.lib)
target_link_libraries(OgrePosteffects optimized  ${OGRE_OIS_LIB_REL}/OIS.lib)
 
target_link_libraries(OgrePosteffects debug ${OGRE_LIBS_DIR_DBG}/OgreMain_d.lib)
target_link_libraries(OgrePosteffects debug ${OGRE_LIBS_DIR_DBG}/OgreOverlay_d.lib)

target_link_libraries(OgrePosteffects optimized ${OGRE_LIBS_DIR_REL}/OgreMain.lib)
target_link_libraries(OgrePosteffects optimized ${OGRE_LIBS_DIR_REL}/OgreOverlay.lib)

# Install project
if(WIN32)

	install(FILES ${OGRE_BIN_DIR_REL}/OgreMain.dll
		${OGRE_BIN_DIR_REL}/RenderSystem_GL.dll
        ${OGRE_BIN_DIR_REL}/RenderSystem_Direct3D9.dll
        ${OGRE_BIN_DIR_REL}/OgreOverlay.dll
		${OGRE_OIS_BIN_REL}/OIS.dll
        ${OGRE_BIN_DIR_REL}/Plugin_ParticleFX.dll
		DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/"
		CONFIGURATIONS Release 
	)
 
	install(FILES ${OGRE_BIN_DIR_DBG}/OgreMain_d.dll
		${OGRE_BIN_DIR_DBG}/RenderSystem_GL_d.dll
        ${OGRE_BIN_DIR_DBG}/RenderSystem_Direct3D9_d.dll
        ${OGRE_BIN_DIR_DBG}/OgreOverlay_d.dll
		${OGRE_OIS_BIN_DBG}/OIS_d.dll
        ${OGRE_BIN_DIR_DBG}/Plugin_ParticleFX_d.dll
		DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/"
		CONFIGURATIONS Debug
	)
endif()