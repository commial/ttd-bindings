############################################
## Create Static library for ttd bindings ##
############################################

set(TTD_SRC
	TTD.cpp
	sha256.cpp
	utils.cpp
)

set(TTD_INCLUDE
	TTD.hpp
	sha256.h
	utils.h
	csts.h
)

add_library(ttd_static STATIC ${TTD_INCLUDE} ${TTD_SRC})
target_compile_features(ttd_static PRIVATE cxx_std_17)

target_include_directories(
	ttd_static
	INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(
	/wd4267
	/wd4244
	/wd4099
)
