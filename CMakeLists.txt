cmake_minimum_required(VERSION 3.20)

set(PROJECT ShaderOpt)

project(${PROJECT} CXX)
enable_language(C CXX)

## for group smake targets in the dir CmakeTargets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CmakeTargets")

## show the dependency graph of cmake
## set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}/debug")
set(CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}/release")
set(CMAKE_PDB_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}/minsizerel")
set(CMAKE_PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}/reldeb")

set(CMAKE_INSTALL_PREFIX ${FINAL_BIN_DIR})
set(CMAKE_INSTALL_BINDIR ${FINAL_BIN_DIR})

set(USE_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(ARCH x64)

if(NOT CMAKE_DEBUG_POSTFIX)
	set(CMAKE_DEBUG_POSTFIX _debug)
endif()

if(NOT CMAKE_RELEASE_POSTFIX)
	set(CMAKE_RELEASE_POSTFIX) ## to config in file
endif()

if(NOT CMAKE_MINSIZEREL_POSTFIX)
	set(CMAKE_MINSIZEREL_POSTFIX _minsizerel)
endif()

if(NOT CMAKE_RELWITHDEBINFO_POSTFIX)
	set(CMAKE_RELWITHDEBINFO_POSTFIX _reldeb)
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zo")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
	add_definitions(-DMSVC)
	add_definitions(-DNOMINMAX)
	add_definitions(-D_USE_MATH_DEFINES)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -m64 -Wall -Wextra -Wpedantic -Wunused-parameter -Wno-unknown-pragmas")
endif ()

if(WIN32)
	add_definitions(-DWIN32)
endif()

include(cmake/ezlibs.cmake)
include(cmake/glslang.cmake)

# App Sources
file(GLOB_RECURSE SRC_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.* 
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.*
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_SOURCES})

# cmake Sources for have it the visual studio tree of the App
file(GLOB_RECURSE CMAKE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/cmake PREFIX cmake FILES ${CMAKE_SOURCES})

add_library(${PROJECT} 
	${SRC_SOURCES}
	${CMAKE_SOURCES}
	${EZ_LIBS_SOURCES}
)

set(SHADER_OPT_INCLUDES 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${GLSLANG_INCLUDE_DIRS}
	${EZLIBS_INCLUDE_DIR}
)
set(SHADER_OPT_INCLUDES ${SHADER_OPT_INCLUDES} PARENT_SCOPE)
target_include_directories(${PROJECT} PRIVATE ${SHADER_OPT_INCLUDES})

target_link_libraries(${PROJECT} 
	${GLSLANG_LIBRARIES}
)

set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}_${ARCH}")

set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}")
