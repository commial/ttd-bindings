#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include <codecvt>
#include <set>
#include <fstream>
#include <mutex>
#include "TTD.hpp"

std::set<TTD::GuestAddress> reached_addrs;
std::mutex set_access;

bool memCallback(unsigned __int64 callback_value, TTD::TTD_Replay_MemoryWatchpointResult* mem, struct TTD::TTD_Replay_IThreadView* thread_info) {
	set_access.lock();
	reached_addrs.insert(mem->addr);
	set_access.unlock();

	int size = reached_addrs.size();
	if (size % 0x1000 == 0) {
		TTD::Position* current = thread_info->IThreadView->GetPosition(thread_info);
		printf("Number of entry: 0x%llx, current position %llx:%llx\n", size, current->Major, current->Minor);
	}
	return FALSE;
}

/**** ARGUMENT PARSING ****/

// From https://stackoverflow.com/a/868894
char* getCmdOption(char** begin, char** end, const std::string& option)
{
	char** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
	return std::find(begin, end, option) != end;
}

std::vector<std::wstring> extractModules(char* mod_arg) {
	std::vector<std::wstring> mod_names;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	std::string mod_arg_str(mod_arg);
	auto start = 0U;
	auto end = mod_arg_str.find(",");
	while (end != std::string::npos)
	{
		mod_names.push_back(converter.from_bytes(mod_arg_str.substr(start, end - start)));
		start = end + 1;
		end = mod_arg_str.find(",", start);
	}
	mod_names.push_back(converter.from_bytes(mod_arg_str.substr(start, end)));
	return mod_names;
}

std::wstring pathToName(wchar_t* path_in) {
	std::wstring path(path_in);
	auto name = path.substr(path.find_last_of(L"/\\") + 1);
	return name.substr(0, name.find_last_of(L"."));
}

/**** END ARGUMENT PARSING ****/

int main(int argc, char** argv)
{
	if (argc <= 1) {
		printf("Usage:\n");
		printf("%s [options] trace\n", argv[0]);
		printf("Options:\n");
		printf("-b\tStarting position, as 1234:56. If not set, use the first position\n");
		printf("-e\tEnding position, as 1234:56. If not set, use the last position\n");
		printf("-m\tModules to cover, as 'ntdll,cmd'. If not set, list availables modules and exit\n");
		return 0;
	}

	TTD::ReplayEngine ttdengine = TTD::ReplayEngine();
	int result;

	std::cout << "Openning the trace\n";
	wchar_t trace_path[MAX_PATH] = { 0 };
	StringCbPrintfW(trace_path, MAX_PATH, L"%S", argv[argc - 1]);
	result = ttdengine.Initialize(trace_path);

	if (result == 0) {
		std::wcerr << "Fail to open the trace";
		exit(-1);
	}

	TTD::Position* first = ttdengine.GetFirstPosition();
	TTD::Position* last = ttdengine.GetLastPosition();
	TTD::Cursor ttdcursor = ttdengine.NewCursor();

	char* begin_arg = getCmdOption(argv, argv + argc, "-b");
	if (begin_arg) {
		unsigned __int64 Major, Minor;
		sscanf_s(begin_arg, "%llx:%llx", &Major, &Minor);
		ttdcursor.SetPosition(Major, Minor);
	}
	else {
		ttdcursor.SetPosition(first);
	}

	TTD::Position end;
	char* end_arg = getCmdOption(argv, argv + argc, "-e");
	if (end_arg) {
		sscanf_s(end_arg, "%llx:%llx", &end.Major, &end.Minor);
	}
	else {
		end.Major = last->Major;
		end.Minor = last->Minor;
	}

	char* mod_arg = getCmdOption(argv, argv + argc, "-m");
	if (!mod_arg) {
		std::cout << "Module:\n";
		std::cout << ttdengine.GetModuleCount() << "\n";

		std::cout << "ModuleList:\n";
		const TTD::TTD_Replay_Module* mod_list = ttdengine.GetModuleList();
		for (int i = 0; i < ttdengine.GetModuleCount(); i++) {
			printf("%llx\t%llx\t%ls\n", mod_list[i].base_addr, mod_list[i].imageSize, mod_list[i].path);
		}
		exit(0);
	}
	auto asked = extractModules(mod_arg);
	const TTD::TTD_Replay_Module* mod_list = ttdengine.GetModuleList();
	for (int i = 0; i < ttdengine.GetModuleCount(); i++) {
		auto name_no_dll = pathToName(mod_list[i].path);
		if (std::find(asked.begin(), asked.end(), name_no_dll) != asked.end()) {
			printf("Tracking %ls\n", name_no_dll.c_str());

			// Add execution BP for [BaseAddr; BaseAddr + Size[
			TTD::TTD_Replay_MemoryWatchpointData data;
			data.addr = mod_list[i].base_addr;
			data.size = mod_list[i].imageSize;
			data.flags = TTD::BP_FLAGS::EXEC;
			ttdcursor.AddMemoryWatchpoint(&data);
		}
	}

	// Enable memory tracking
	ttdcursor.SetMemoryWatchpointCallback((TTD::PROC_MemCallback)memCallback, 0);

	// Launch the replay
	TTD::TTD_Replay_ICursorView_ReplayResult replayrez;
	ttdcursor.ReplayForward(&replayrez, &end, -1);

	// Create the corresponding traces
	printf("Got 0x%llx addresses\n", reached_addrs.size());

	// This could be optimized to browse only once the vector... but comparing to I/O, it's fast enough
	for (int i = 0; i < ttdengine.GetModuleCount(); i++) {
		auto name_no_dll = pathToName(mod_list[i].path);
		if (std::find(asked.begin(), asked.end(), name_no_dll) != asked.end()) {
			wchar_t file_path[MAX_PATH] = { 0 };
			StringCbPrintfW(file_path, MAX_PATH, L"%s.trace.txt", name_no_dll.c_str());
			printf("Dump to file %ls\n", file_path);
			std::wofstream fdesc;
			fdesc.open(file_path, std::ios::out);
			auto itaddr = reached_addrs.begin();
			while (itaddr != reached_addrs.end()) {
				if (*itaddr >= mod_list[i].base_addr && *itaddr < (mod_list[i].base_addr + mod_list[i].imageSize)) {
					fdesc << name_no_dll << "+" << std::hex << (*itaddr - mod_list[i].base_addr) << std::endl;
				}
				itaddr++;
			}
			fdesc.close();
		}
	}
	return 0;
}
