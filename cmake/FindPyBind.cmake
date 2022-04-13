#############################################
# Find PyBind use to create python bindings #
#############################################

if(PyBind_SOURCE_DIR)
	find_path(PyBind_SOURCE_ROOT
	NAMES include/pybind11/pybind11.h
	PATHS "${PyBind_SOURCE_DIR}"
	NO_DEFAULT_PATH)
else()
	set(PyBind_SOURCE_ROOT PyBind_SOURCE_ROOT-NOTFOUND)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    PyBind
	REQUIRED_VARS PyBind_SOURCE_ROOT
    FAIL_MESSAGE "
#######################################################
Could not find PyBind11 headers. Make sure PyBind_SOURCE_DIR is set to the root of the PyBind11 source repository.
#######################################################
")

mark_as_advanced(PyBind_MAIN_HEADER)

if(PyBind_FOUND)
	set(PyBind_INCLUDE_DIRS "${PyBind_SOURCE_ROOT}/include")
endif()
