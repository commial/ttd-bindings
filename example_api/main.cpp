#include <windows.h> 
#include <stdio.h> 
#include <iostream>
#include "utils.h"
#include "TTD.hpp"

/*
@callback_value: value passed at callback registering
2 cases:
	@addr_func: called function address
	@addr_ret: return address after the call
OR
	@addr_ret is 0
	@addr_func is the current returned address
*/
void callCallback(unsigned __int64 callback_value, TTD::GuestAddress addr_func, TTD::GuestAddress addr_ret, struct TTD::TTD_Replay_IThreadView* thread_info) {
	printf("[CALL CALLBACK] ");
	printf("arg1: %llx, arg2: %llx, arg3: %llx, arg4: %llx\n", callback_value, addr_func, addr_ret, thread_info);
	TTD::Position* current = thread_info->IThreadView->GetPosition(thread_info);
	printf("Program counter: %llx | Position: %llx:%llx\n", thread_info->IThreadView->GetProgramCounter(thread_info), current->Major, current->Minor);
	if (addr_ret == 0) {
		printf("Returned value: %llx\n", thread_info->IThreadView->GetBasicReturnValue(thread_info));
	}
	return;
}

/*
* @callback_value: value passed at callback registering
* Returns TRUE to stop execution on break
*/
bool memCallback(unsigned __int64 callback_value, TTD::TTD_Replay_MemoryWatchpointResult* mem, struct TTD::TTD_Replay_IThreadView* thread_info) {
	printf("[MEM CALLBACK] ");
	printf("callback_value: %llx, guest_addr: %llx, size: %llx, flags: %llx\n", callback_value, mem->addr, mem->size, mem->flags);
	TTD::Position* current = thread_info->IThreadView->GetPosition(thread_info);
	printf("Program counter: %llx | Position: %llx:%llx\n", thread_info->IThreadView->GetProgramCounter(thread_info), current->Major, current->Minor);
	return TRUE;
}

int main()
{
	TTD::ReplayEngine ttdengine = TTD::ReplayEngine();
	int result;

	std::cout << "Openning the trace\n";
	result = ttdengine.Initialize(L"D:\\traces\\lsm_rpc.run");
	if (result == 0) {
		std::cout << "Fail to open the trace";
		exit(-1);
	}

	std::cout << "System Info:\n";
	TTD::TTD_SystemInfo* info = ttdengine.GetSystemInfo();
	DumpHex(info, 0x100); // size is not correct

	std::cout << "Thread count:\n";
	std::cout << ttdengine.GetThreadCount() << "\n";

	std::cout << "First Position:\n";
	TTD::Position* first = ttdengine.GetFirstPosition();
	printf("%llx:%llx\n", first->Major, first->Minor);

	std::cout << "Last Position:\n";
	TTD::Position* last = ttdengine.GetLastPosition();
	printf("%llx:%llx\n", last->Major, last->Minor); 

	std::cout << "Peb:\n";
	printf("%llx\n", ttdengine.GetPebAddress());

	std::cout << "New Cursor:\n";
	TTD::Cursor ttdcursor = ttdengine.NewCursor();

	std::cout << "\nSetPosition:\n";
	ttdcursor.SetPosition(first);

	std::cout << "\nThread count cursor:\n";
	std::cout << ttdcursor.GetThreadCount();

	std::cout << "\nProgram counter\n";
	printf("%llx\n", ttdcursor.GetProgramCounter());

	std::cout << "\nThread ID:\n";
	printf("%x\n", ttdcursor.GetThreadInfo()->threadid);
	DumpHex(ttdcursor.GetThreadInfo(), 0x20);

	std::cout << "\nSet position to F3:0\n";
	ttdcursor.SetPosition(0xF3, 0);
	std::cout << "\nProgram counter\n";
	printf("%llx\n", ttdcursor.GetProgramCounter());

	std::cout << "\nThread ID:\n";
	printf("%x\n", ttdcursor.GetThreadInfo()->threadid);
	DumpHex(ttdcursor.GetThreadInfo(), 0x20);

	std::cout << "\nContext:\n";
    // Use  ttdcursor.GetContextx86() for x86 context
    auto ctxt = ttdcursor.GetContextx86_64();
	DumpHex(ctxt, 0xA70);
	printf("RCX: %llx\n", ctxt->Rcx);
	std::cout << "Query memory @rcx:\n";
	struct TTD::MemoryBuffer* memorybuffer = ttdcursor.QueryMemoryBuffer(ctxt->Rcx, 0x30);
	if (memorybuffer->data == NULL) {
		printf("Query Memory fail: memory do not exists in trace");
	}
	printf("%llx: ", memorybuffer->addr);
	DumpHex(memorybuffer->data, memorybuffer->size);
	free(memorybuffer->data);
	free(memorybuffer);

	TTD::TTD_Replay_ICursorView_ReplayResult replayrez;
	for (int i = 0; i < 3; i++) {
		std::cout << "Step forward\n";
		ttdcursor.ReplayForward(&replayrez, last, 1);
		DumpHex(&replayrez, sizeof(TTD::TTD_Replay_ICursorView_ReplayResult));
		std::cout << "\nProgram counter\n";
		printf("%llx\n", ttdcursor.GetProgramCounter());
	}
	std::cout << "Step forward step=2\n";
	ttdcursor.ReplayForward(&replayrez, last, 2);
	DumpHex(&replayrez, sizeof(TTD::TTD_Replay_ICursorView_ReplayResult));
	std::cout << "\nProgram counter\n";
	printf("%llx\n", ttdcursor.GetProgramCounter());

	std::cout << "Step forward MAX\n";
	ttdcursor.ReplayForward(&replayrez, last, -1);
	DumpHex(&replayrez, sizeof(TTD::TTD_Replay_ICursorView_ReplayResult));
	std::cout << "\nProgram counter\n";
	printf("%llx\n", ttdcursor.GetProgramCounter());

	std::cout << "Step backward 1\n";
	ttdcursor.ReplayBackward(&replayrez, first, 1);
	DumpHex(&replayrez, sizeof(TTD::TTD_Replay_ICursorView_ReplayResult));
	std::cout << "\nProgram counter\n";
	printf("%llx\n", ttdcursor.GetProgramCounter());

	std::cout << "\nSet position to first instr\n";
	ttdcursor.SetPosition(first);

	std::cout << "Add call callback";
	ttdcursor.SetCallReturnCallback((TTD::PROC_CallCallback) callCallback, 0);

	std::cout << "Step 100 instr\n";
	ttdcursor.ReplayForward(&replayrez, last, 100);
	DumpHex(&replayrez, sizeof(TTD::TTD_Replay_ICursorView_ReplayResult));
	std::cout << "\nProgram counter\n";
	printf("%llx\n", ttdcursor.GetProgramCounter());

	std::cout << "Module:\n";
	std::cout << ttdengine.GetModuleCount() << "\n";

	std::cout << "ModuleList:\n";
	const TTD::TTD_Replay_Module* mod_list = ttdengine.GetModuleList();
	for (int i = 0; i < ttdengine.GetModuleCount(); i++) {
		printf("%llx\t%llx\t%ls\n", mod_list[i].base_addr, mod_list[i].imageSize, mod_list[i].path);
	}

	std::cout << "ExceptionList:" << std::endl;
	for (const auto& exceptionEvent : ttdengine.GetExceptionEvents())
	{
		std::cout << "Exception raised at : 0x" << std::hex << exceptionEvent.info.ExceptionAddress << std::endl;
	}

	std::cout << "Remove callback\n";
	ttdcursor.SetCallReturnCallback(0, 0);
	std::cout << "Breakpoint r4 at 0x1c26ac21e40 from beginning\n";
	TTD::TTD_Replay_MemoryWatchpointData data;
	data.addr = 0x1c26ac21e40;
	data.size = 4;
	data.flags = TTD::BP_FLAGS::READ;
	ttdcursor.AddMemoryWatchpoint(&data);
	ttdcursor.SetMemoryWatchpointCallback((TTD::PROC_MemCallback)memCallback, 1234);
	ttdcursor.ReplayForward(&replayrez, last, -1);
	printf("%llx\n", ttdcursor.GetProgramCounter());
	ttdcursor.RemoveMemoryWatchpoint(&data);

	std::cout << "Breakpoint X at 0x7fffcf045418 from beginning\n";
	data.addr = 0x7fffcf045418;
	data.size = 1;
	data.flags = TTD::BP_FLAGS::EXEC;
	ttdcursor.AddMemoryWatchpoint(&data);
	ttdcursor.ReplayForward(&replayrez, last, -1);
	printf("%llx\n", ttdcursor.GetProgramCounter());

	// Exemple of timeline
	std::cout << "Timeline" << std::endl;
	auto moduleLoaded = ttdengine.GetModuleLoadedEvents();
	auto moduleUnloaded = ttdengine.GetModuleUnloadedEvents();
	auto threadCreate = ttdengine.GetThreadCreatedEvents();
	auto threadTerminate = ttdengine.GetThreadTerminatedEvents();

	auto itModuleLoaded = moduleLoaded.begin();
	auto itModuleUnloaded = moduleUnloaded.begin();
	auto itThreadCreate = threadCreate.begin();
	auto itThreadTerminate = threadTerminate.begin();

	while (itModuleLoaded != moduleLoaded.end() || itModuleUnloaded != moduleUnloaded.end() || itThreadCreate != threadCreate.end() || itThreadTerminate != threadTerminate.end())
	{
		TTD::Position moduleLoadedPosition = (itModuleLoaded == moduleLoaded.end()) ? TTD::POSITION_MAX : itModuleLoaded->pos;
		TTD::Position moduleUnloadedPosition = (itModuleUnloaded == moduleUnloaded.end()) ? TTD::POSITION_MAX : itModuleUnloaded->pos;
		TTD::Position threadCreatePosition = (itThreadCreate == threadCreate.end()) ? TTD::POSITION_MAX : itThreadCreate->pos;
		TTD::Position threadTerminatePosition = (itThreadTerminate == threadTerminate.end()) ? TTD::POSITION_MAX : itThreadTerminate->pos;

 		if (moduleLoadedPosition < moduleUnloadedPosition && moduleLoadedPosition < threadCreatePosition && moduleLoadedPosition < threadTerminatePosition)
		{
			std::wcout << "Module Loaded at " << std::hex << itModuleLoaded->pos.Major << ":" << std::hex << itModuleLoaded->pos.Minor << " : " << std::wstring(itModuleLoaded->info->path) << std::endl;
			itModuleLoaded++;
		}

		if (moduleUnloadedPosition < moduleLoadedPosition && moduleUnloadedPosition < threadCreatePosition && moduleUnloadedPosition < threadTerminatePosition)
		{
			std::wcout << "Module Unloaded at " << std::hex << itModuleUnloaded->pos.Major << ":" << std::hex << itModuleUnloaded->pos.Minor << " : " << std::wstring(itModuleUnloaded->info->path) << std::endl;
			itModuleUnloaded++;
		}

		if (threadCreatePosition < moduleLoadedPosition && threadCreatePosition < moduleLoadedPosition && threadCreatePosition < threadTerminatePosition)
		{
			std::wcout << "Thread Created at " << std::hex << itThreadCreate->pos.Major << ":" << std::hex << itThreadCreate->pos.Minor << " id : " << std::hex << itThreadCreate->info->threadid << std::endl;
			itThreadCreate++;
		}

		if (threadTerminatePosition < moduleLoadedPosition && threadTerminatePosition < moduleLoadedPosition && threadTerminatePosition < threadCreatePosition)
		{
			std::wcout << "Thread Terminated at " << std::hex << itThreadTerminate->pos.Major << ":" << std::hex << itThreadTerminate->pos.Minor << " id : " << std::hex << itThreadTerminate->info->threadid << std::endl;
			itThreadTerminate++;
		}
	}
}
