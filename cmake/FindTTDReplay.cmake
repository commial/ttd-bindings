############################################
# Use to find dll of Time Travel Debugging #
############################################
# OUTPUT is ${TTDReplay_INSTALL_PATH} set with the correct path
# INSTALL_TTDREPLAY macro use to copy ttd dll files

set(WindbgPreview_INSTALL_PATH "$ENV{ProgramFiles}/WindowsApps/Microsoft.WinDbg_1.2202.7001.0_neutral__8wekyb3d8bbwe/amd64/ttd")

find_path(
	TTDReplay_INSTALL_PATH
	NAMES TTDReplay.dll
	PATHS "${WindbgPreview_INSTALL_PATH}"
	NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    TTDReplay
	REQUIRED_VARS TTDReplay_INSTALL_PATH
    FAIL_MESSAGE "
#######################################################
Could not find Time Travel Debugging dll. Make sure you installed windbg preview edition or disable using -DINSTALL_TTDDLL=OFF
#######################################################
")

macro(INSTALL_TTDREPLAY TARGET_NAME)

add_custom_command(
	TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TTDReplay_INSTALL_PATH}/TTDReplay.dll" $<TARGET_FILE_DIR:${TARGET_NAME}>/TTDReplay.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TTDReplay_INSTALL_PATH}/TTDReplayCPU.dll" $<TARGET_FILE_DIR:${TARGET_NAME}>/TTDReplayCPU.dll
)

endmacro(INSTALL_TTDREPLAY)

