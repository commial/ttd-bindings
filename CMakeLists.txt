# Minimum CMake required
cmake_minimum_required(VERSION 3.11)

# Project
project(ttd-bindings LANGUAGES CXX VERSION 1.0.0)

# Opions
option(BUILD_EXAMPLES "Build examples programs" OFF)
option(BUILD_PYTHON_BINDINGS "Build python bindings" OFF)
option(INSTALL_TTDDLL "Try to find and install ttd dll from Windbg Preview install folder" ON)

# Config
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if (MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /WX")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /WX")
endif(MSVC)

# Add cmake folder for 
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(TTD)

# Tests
if(BUILD_EXAMPLES)
    add_subdirectory(example_api)
    add_subdirectory(example_calltree)
    add_subdirectory(example_diff)
endif(BUILD_EXAMPLES)

if(BUILD_PYTHON_BINDINGS)
	add_subdirectory(python-bindings)
endif(BUILD_PYTHON_BINDINGS)

# Summary
message(STATUS "Configuration summary")
message(STATUS "Project name                 : ${PROJECT_NAME}")
message(STATUS "Project version              : ${PROJECT_VERSION}")
message(STATUS "Build Examples             	 : ${BUILD_EXAMPLES}")
message(STATUS "Build Python Bindings      	 : ${BUILD_PYTHON_BINDINGS}")
message(STATUS "Install ttd dlls    	 	 : ${INSTALL_TTDDLL}")
