#############################################
############ simple API example #############
#############################################
set(EXAMPLE_API_SRC
	main.cpp
)


add_executable(example_api ${EXAMPLE_API_SRC})
target_compile_features(example_api PRIVATE cxx_std_17)
target_link_libraries(example_api ttd_static)

add_definitions(
	/wd4477
)

# Install ttd dlls if it's requested
if (INSTALL_TTDDLL)
	find_package(TTDReplay)
	INSTALL_TTDREPLAY(example_api)
endif (INSTALL_TTDDLL)