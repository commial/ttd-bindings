#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <iostream>
#include <iomanip>
#include <codecvt>
#include <set>
#include <fstream>
#include <mutex>
#include "TTD.hpp"

// Track memory accesses between two trace lines
std::vector<TTD::TTD_Replay_MemoryWatchpointResult*> memsinfo;

/* Clean-up memsinfo */
void cleanMemsinfo() {
	for (auto itMems = memsinfo.begin(); itMems != memsinfo.end(); itMems++) {
		free(*itMems);
	}
	memsinfo.clear();
}

/* Lock for memsinfo writing in memory callback */
std::mutex set_accessmems;

bool memCallback(unsigned __int64 callback_value, TTD::TTD_Replay_MemoryWatchpointResult* mem, struct TTD::TTD_Replay_IThreadView* thread_info) {
	TTD::TTD_Replay_MemoryWatchpointResult* temp = (TTD::TTD_Replay_MemoryWatchpointResult*) malloc(sizeof(TTD::TTD_Replay_MemoryWatchpointResult));
	memcpy(temp, mem, sizeof(TTD::TTD_Replay_MemoryWatchpointResult));
	set_accessmems.lock();
	memsinfo.push_back(temp);
	set_accessmems.unlock();
	return FALSE;
}

#define DUMP_REGISTER(REG) if (old == NULL || old->REG != newc->REG) { \
if (print_coma) { fdesc << ","; } \
fdesc << #REG "=0x" << std::hex << (newc->REG); \
print_coma = TRUE; \
}

void dumpContext(std::ofstream& fdesc, CONTEXT* old, CONTEXT* newc, TTD::Cursor ttdcursor) {
	/*
	DWORD64 Rax;
	DWORD64 Rcx;
	DWORD64 Rdx;
	DWORD64 Rbx;
	DWORD64 Rsp;
	DWORD64 Rbp;
	DWORD64 Rsi;
	DWORD64 Rdi;
	DWORD64 R8;
	DWORD64 R9;
	DWORD64 R10;
	DWORD64 R11;
	DWORD64 R12;
	DWORD64 R13;
	DWORD64 R14;
	DWORD64 R15;
	DWORD64 Rip;
	*/
	BOOL print_coma = FALSE;
	DUMP_REGISTER(Rax);
	DUMP_REGISTER(Rbx);
	DUMP_REGISTER(Rcx);
	DUMP_REGISTER(Rdx);
	DUMP_REGISTER(Rdi);
	DUMP_REGISTER(Rsi);
	DUMP_REGISTER(Rbp);
	DUMP_REGISTER(Rsp);
	DUMP_REGISTER(R8);
	DUMP_REGISTER(R9);
	DUMP_REGISTER(R10);
	DUMP_REGISTER(R11);
	DUMP_REGISTER(R12);
	DUMP_REGISTER(R13);
	DUMP_REGISTER(R14);
	DUMP_REGISTER(R15);
	DUMP_REGISTER(Rip);

	for (auto itMems = memsinfo.begin(); itMems != memsinfo.end(); itMems++) {
		//printf("\nMEMS:%x:%x:%x\n", (*itMems)->addr, (*itMems)->size, (*itMems)->flags);
		struct TTD::MemoryBuffer* memorybuffer = ttdcursor.QueryMemoryBuffer((*itMems)->addr, (*itMems)->size);
		if (memorybuffer->data == NULL) {
			std::wcerr << "Query Memory FAIL: memory do not exists in trace";
		}
		if ((*itMems)->flags == 1) { // WRITE
			fdesc << ",mw=0x" << std::hex << memorybuffer->addr << ":";
		}
		else if ((*itMems)->flags == 0) { // READ
			fdesc << ",mr=0x" << std::hex << memorybuffer->addr << ":";
		}
		else {
			std::wcerr << "Unexpected: flags " << std::hex << (*itMems)->flags << "\n";
		}
		for (int i = 0; i < (*itMems)->size; i++) {
			fdesc << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (unsigned int)(((unsigned char*)memorybuffer->data)[i]);
		}
		free(memorybuffer->data);
		free(memorybuffer);
	}
	fdesc << "\n";
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
		printf("-m\tModule to cover, as 'ntdll'. If not set, list availables modules and exit\n");
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

	TTD::GuestAddress addr_min = NULL;
	TTD::GuestAddress addr_max = NULL;
	const TTD::TTD_Replay_Module* mod_list = ttdengine.GetModuleList();
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	auto mod_arg_wide = converter.from_bytes(mod_arg);
	for (int i = 0; i < ttdengine.GetModuleCount(); i++) {
		auto name_no_dll = pathToName(mod_list[i].path);
		if (name_no_dll == mod_arg_wide) {
			printf("Track %ls [%llx to %llx]\n", mod_list[i].path, mod_list[i].base_addr, mod_list[i].base_addr + mod_list[i].imageSize - 1);
			addr_min = mod_list[i].base_addr;
			addr_max = mod_list[i].base_addr + mod_list[i].imageSize;
			break;
		}
	}
	if (addr_min == NULL) {
		printf("Unable to find %s", mod_arg);
		exit(0);
	}

	wchar_t file_path[MAX_PATH] = { 0 };
	StringCbPrintfW(file_path, MAX_PATH, L"%s.trace.tenet", mod_arg_wide.c_str());
	printf("Dump to file %ls\n", file_path);
	std::ofstream fdesc;
	fdesc.open(file_path, std::ofstream::binary);

	int i = 0;
	TTD::TTD_Replay_ICursorView_ReplayResult temp_result;
	CONTEXT* old_context = ttdcursor.GetContextx86_64();
	CONTEXT* new_context = NULL;
	// Dump initial state
	dumpContext(fdesc, NULL, old_context, ttdcursor);

	TTD::TTD_Replay_MemoryWatchpointData data;
	data.addr = 0;
	data.size = 0xFFFFFFFFFFFFFFFF;
	data.flags = TTD::BP_FLAGS::WRITE | TTD::BP_FLAGS::READ;
	ttdcursor.AddMemoryWatchpoint(&data);
	ttdcursor.SetMemoryWatchpointCallback((TTD::PROC_MemCallback)memCallback, 0);

	for (;;) {
		ttdcursor.ReplayForward(&temp_result, &end, 1);
		if (temp_result.stepCount < 1) {
			break;
		}

		new_context = ttdcursor.GetContextx86_64();
		if (new_context->Rip < addr_min || new_context->Rip >= addr_max)
			continue;
		dumpContext(fdesc, old_context, new_context, ttdcursor);
		cleanMemsinfo();
		old_context = new_context;

		i++;
		if (i % 0x1000 == 0) {
			printf("0x%x instructions...\n", i);
		}
	}
	fdesc.close();
}
