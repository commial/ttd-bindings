############################################
# Create python bindings library pyTTD.pyd #
############################################

find_package(Python3 COMPONENTS Interpreter Development)
find_package(PyBind)

set(PYTHON_BINDINGS_SRC
	module.cpp
)

add_library(python-bindings SHARED ${PYTHON_BINDINGS_SRC})

target_compile_features(python-bindings PRIVATE cxx_std_17)
target_link_libraries(python-bindings ttd_static ${Python3_LIBRARIES})

target_include_directories(python-bindings PUBLIC
  $<BUILD_INTERFACE:${Python3_INCLUDE_DIRS}>
  $<BUILD_INTERFACE:${PyBind_INCLUDE_DIRS}>
)

set_target_properties(python-bindings PROPERTIES
	INTERPROCEDURAL_OPTIMIZATION ON
	CXX_VISIBILITY_PRESET ON
	VISIBILITY_INLINES_HIDDEN ON
	OUTPUT_NAME "pyTTD"
	SUFFIX ".pyd"
)

if (INSTALL_TTDDLL)
	find_package(TTDReplay)
	INSTALL_TTDREPLAY(python-bindings)
endif (INSTALL_TTDDLL)