cmake_minimum_required(VERSION 3.20)

set(PROJECT ShaderOptimizer)

project(${PROJECT} CXX)
enable_language(C CXX)

option(USE_BUILDING_OF_TESTS "Enable the build of tests" OFF)

## for group smake targets in the dir CmakeTargets
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CmakeTargets")

## show the dependency graph of cmake
## set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DUSE_STD_FILESYSTEM)
add_definitions(-DIMGUI_ENABLE_FREETYPE)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_definitions(-DUSE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS)
add_definitions(-DIMGUI_INCLUDE="${CMAKE_SOURCE_DIR}/3rdparty/imguipack/imguipack.h")
add_definitions(-DIMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/headers/customImGuiConfig.h")
add_definitions(-DCUSTOM_IMWIDGETS_CONFIG="${CMAKE_SOURCE_DIR}/src/headers/customImWidgetsConfig.h")
add_definitions(-DCUSTOM_IN_APP_GPU_PROFILER_CONFIG="${CMAKE_SOURCE_DIR}/src/headers/customInAppGpuProfiler.h")
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/src/headers/customImGuiFileDialogConfig.h")
add_definitions(-DCUSTOM_MESSAGING_CONFIG_HEADER="${CMAKE_SOURCE_DIR}/src/headers/customMessagingConfigHeader.h")
add_definitions(-DCUSTOM_LOG_TO_GRAPH_BACKEND_CONFIG="${CMAKE_SOURCE_DIR}/src/headers/customStrockerBackendConfig.h")
add_definitions(-DCUSTOM_THEME_HELPER_CONFIG_HEADER="${CMAKE_SOURCE_DIR}/src/headers/customImGuiThemeHelperConfigHeader.h")

add_subdirectory(${CMAKE_SOURCE_DIR}/ShaderOpt)

include_directories(${SHADER_OPT_INCLUDES})

include(cmake/3rdparty.cmake)

include_directories(
	${CMAKE_SOURCE_DIR}/3rdparty
	${CMAKE_SOURCE_DIR}/src
	${IMGUIPACK_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}
	${PLOG_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${GLAD_INCLUDE_DIR}
	${IAGP_INCLUDE_DIR}
)

###########################################################
###########################################################
###########################################################

# Assets sources
file(GLOB_RECURSE ASSETS_SOURCES ${CMAKE_SOURCE_DIR}/bin/assets/*.*)
source_group(TREE ${CMAKE_SOURCE_DIR}/bin/assets PREFIX assets FILES ${ASSETS_SOURCES})

# Main sources
file(GLOB MAIN_SOURCES ${CMAKE_SOURCE_DIR}/main.cpp)
source_group(main FILES ${MAIN_SOURCES})

# App Sources
file(GLOB_RECURSE SRC_SOURCES ${CMAKE_SOURCE_DIR}/src/*.*)
source_group(TREE ${CMAKE_SOURCE_DIR}/src PREFIX src FILES ${SRC_SOURCES})

# Doc Sources
file(GLOB_RECURSE DOC_SOURCES ${CMAKE_SOURCE_DIR}/doc/*.*)
source_group(TREE ${CMAKE_SOURCE_DIR}/doc PREFIX doc FILES ${DOC_SOURCES})
set_property(SOURCE ${DOC_SOURCES} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")

# cmake Sources for have it the visual studio tree of the App
file(GLOB_RECURSE PROJECT_CMAKE_FILES ${CMAKE_SOURCE_DIR}/cmake/*.cmake)
source_group(TREE ${CMAKE_SOURCE_DIR}/cmake PREFIX cmake FILES ${PROJECT_CMAKE_FILES})

set(MAIN_SOURCES)
set(IMGUI_IMPL_SOURCES)

file(GLOB MAIN_SOURCES ${CMAKE_SOURCE_DIR}/main.cpp)
file(GLOB IMGUI_IMPL_SOURCES 
	${CMAKE_SOURCE_DIR}/3rdparty/imguipack/3rdparty/imgui_docking/backends/imgui_impl_glfw.cpp
	${CMAKE_SOURCE_DIR}/3rdparty/imguipack/3rdparty/imgui_docking/backends/imgui_impl_glfw.h
	${CMAKE_SOURCE_DIR}/3rdparty/imguipack/3rdparty/imgui_docking/backends/imgui_impl_opengl3.cpp
	${CMAKE_SOURCE_DIR}/3rdparty/imguipack/3rdparty/imgui_docking/backends/imgui_impl_opengl3.h
	${CMAKE_SOURCE_DIR}/3rdparty/imguipack/3rdparty/imgui_docking/misc/freetype/imgui_freetype.cpp
	${CMAKE_SOURCE_DIR}/3rdparty/imguipack/3rdparty/imgui_docking/misc/freetype/imgui_freetype.h)

source_group(src FILES ${MAIN_SOURCES})
source_group(src/ImGuiImpl FILES ${IMGUI_IMPL_SOURCES})

set(SOURCES 
	${SRC_SOURCES}
	${MAIN_SOURCES}
	${ASSETS_SOURCES}
	${IMGUI_IMPL_SOURCES}
	${PROJECT_CMAKE_FILES}
)

add_executable(${PROJECT} ${SOURCES})

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

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
set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}_${ARCH}")
target_link_libraries(${PROJECT}
	${IMGUIPACK_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${GLSLANG_LIBRARIES}
	${OPENGL_LIBRARIES}
	${GLFW_LIBRARIES}
	${GLAD_LIBRARIES}
	ShaderOpt
)

set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${FINAL_BIN_DIR}")

########################################################
################# BUILD INCREMENT TOOL #################
########################################################

## only for Debug
add_subdirectory(3rdparty/buildinc)
add_custom_command(
		TARGET ${PROJECT} PRE_BUILD
		COMMAND BuildInc ${PROJECT} $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/src/headers/${PROJECT}Build.h> -ff ${CMAKE_SOURCE_DIR}/src/res/big.flf --no-help
)

########################################################
##### TESTS ############################################
########################################################

if(USE_BUILDING_OF_TESTS)
	enable_testing()
endif()
