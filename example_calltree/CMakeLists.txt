set(EXAMPLE_CALLTREE_SRC
	main.cpp
)


add_executable(example_calltree ${EXAMPLE_CALLTREE_SRC})
target_compile_features(example_calltree PRIVATE cxx_std_17)
target_link_libraries(example_calltree ttd_static)

add_definitions(
	/wd4477
	/wd4267
)

# Install ttd dlls if it's requested
if (INSTALL_TTDDLL)
	find_package(TTDReplay)
	INSTALL_TTDREPLAY(example_calltree)
endif(INSTALL_TTDDLL)
