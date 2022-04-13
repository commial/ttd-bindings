#############################################
############# Diff API example ##############
#############################################
set(EXAMPLE_DIFF_SRC
	main.cpp
)


add_executable(example_diff ${EXAMPLE_DIFF_SRC})
target_compile_features(example_diff PRIVATE cxx_std_17)
target_link_libraries(example_diff ttd_static)

add_definitions(
	/wd4477
	/wd4267
)

# Install ttd dlls if it's requested
if (INSTALL_TTDDLL)
	find_package(TTDReplay)
	INSTALL_TTDREPLAY(example_diff)
endif(INSTALL_TTDDLL)