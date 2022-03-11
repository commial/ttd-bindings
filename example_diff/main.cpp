#include <windows.h> 
#include <stdio.h> 
#include <iostream>
#include <vector>
#include "TTD.hpp"

/*
@callback_value: value passed at callback registering
@addr_func: called function address
@addr_ret: return address after the call
*/
TTD::GuestAddress g_last_func;
TTD::GuestAddress g_next_ret;
TTD::GuestAddress g_last_func2;
TTD::GuestAddress g_next_ret2;
void callCallback_diff(unsigned __int64 callback_value, TTD::GuestAddress addr_func, TTD::GuestAddress addr_ret, struct TTD::TTD_Replay_IThreadView* arg4) {
	if (callback_value == 1) {
		g_last_func = addr_func;
		g_next_ret = addr_ret;
	}
	else if (callback_value == 2) {
		g_last_func2 = addr_func;
		g_next_ret2 = addr_ret;
	}
	return;
}


struct Snapshot {
	TTD::GuestAddress save_last_func;
	TTD::GuestAddress save_last_func2;
	TTD::GuestAddress save_next_ret;
	TTD::GuestAddress save_next_ret2;
	TTD::Position cur;
	TTD::Position cur2;
};
struct Snapshot g_snapshot;

void takeSnapshot(TTD::Cursor* cursor, TTD::Cursor* cursor2) {
	g_snapshot.save_last_func = g_last_func;
	g_snapshot.save_last_func2 = g_last_func2;
	g_snapshot.save_next_ret = g_next_ret;
	g_snapshot.save_next_ret2 = g_next_ret2;
	memcpy(&g_snapshot.cur, cursor->GetPosition(), sizeof(TTD::Position));
	memcpy(&g_snapshot.cur2, cursor2->GetPosition(), sizeof(TTD::Position));
}

void restoreSnapshot(TTD::Cursor* cursor, TTD::Cursor* cursor2) {
	cursor->SetPosition(&g_snapshot.cur);
	cursor2->SetPosition(&g_snapshot.cur2);
	g_last_func = g_snapshot.save_last_func;
	g_last_func2 = g_snapshot.save_last_func2;
	g_next_ret = g_snapshot.save_next_ret;
	g_next_ret2 = g_snapshot.save_next_ret2;
}

const unsigned __int64 DEFAULT_STEP_SIZE = 10000;
const std::vector<TTD::GuestAddress> KNOWN_FUNC = {
	0x7ffb10ec1af0 // RtlHeapAlloc
};
int main()
{
	TTD::ReplayEngine ttdengine = TTD::ReplayEngine();
	TTD::ReplayEngine ttdengine2 = TTD::ReplayEngine();
	int result;
	unsigned __int64 step_size = DEFAULT_STEP_SIZE;

	std::cout << "Openning the trace\n";
	result = ttdengine.Initialize(L"D:\\traces\\lsass01.run");
	if (result == 0) {
		std::cout << "Fail to open the trace";
		exit(-1);
	}
	std::cout << "Openning the trace2\n";
	result = ttdengine2.Initialize(L"D:\\traces\\lsass01.run");
	if (result == 0) {
		std::cout << "Fail to open the trace";
		exit(-1);
	}

	TTD::Cursor ttdcursor = ttdengine.NewCursor();
	TTD::Position* last = ttdengine.GetLastPosition();
	TTD::Cursor ttdcursor2 = ttdengine2.NewCursor();
	TTD::Position* last2 = ttdengine2.GetLastPosition();

	ttdcursor.SetCallReturnCallback((TTD::PROC_CallCallback) callCallback_diff, 1);
	ttdcursor2.SetCallReturnCallback((TTD::PROC_CallCallback) callCallback_diff, 2);
	ttdcursor.SetPosition(0x177B, 0x3F);
	ttdcursor2.SetPosition(0x6871, 0x3F);

	TTD::GuestAddress pc;
	TTD::GuestAddress pc2;
	TTD::TTD_Replay_ICursorView_ReplayResult replayrez;
deb:
	takeSnapshot(&ttdcursor, &ttdcursor2);
	TTD::Position* cur = ttdcursor.GetPosition();
	TTD::Position* cur2 = ttdcursor2.GetPosition();

	while (step_size > 0) {
		printf("STEP %d\n", step_size);
		cur = ttdcursor.GetPosition();
		cur2 = ttdcursor2.GetPosition();
		printf("Trace 1: %llx:%llx\t| Trace 2: %llx:%llx\n", cur->Major, cur->Minor, cur2->Major, cur2->Minor);
		ttdcursor.ReplayForward(&replayrez, last, step_size);
		ttdcursor2.ReplayForward(&replayrez, last2, step_size);
		pc = ttdcursor.GetProgramCounter();
		pc2 = ttdcursor2.GetProgramCounter();
		printf("Trace 1 ends on %llx\t| Trace 2 ends on %llx\n", pc, pc2);
		if (pc != pc2) {
			printf("Rollback...");
			restoreSnapshot(&ttdcursor, &ttdcursor2);
			printf("OK\n");
			step_size /= 2;
		}
		else {
			// Set new reference
			takeSnapshot(&ttdcursor, &ttdcursor2);
		}
	}

	printf("Last known reference: Trace 1: %llx:%llx\t| Trace 2: %llx:%llx\n", cur->Major, cur->Minor, cur2->Major, cur2->Minor);
	printf("Cur func: %llx -> %llx | %llx -> %llx\n", g_last_func, g_next_ret, g_last_func2, g_next_ret2);
	if (g_next_ret == g_next_ret2) {
		printf("Both functions ends on the same address: %llx\n", g_next_ret);
		if (std::find(KNOWN_FUNC.begin(), KNOWN_FUNC.end(), g_last_func) != KNOWN_FUNC.end()) { // Function known for path diff, such as allocators
			printf("-> Resync traces after the RET");
			TTD::TTD_Replay_MemoryWatchpointData data;
			data.addr = g_next_ret;
			data.flags = TTD::BP_FLAGS::EXEC;
			data.size = 1;
			ttdcursor.AddMemoryWatchpoint(&data);
			ttdcursor.ReplayForward(&replayrez, last, -1);
			ttdcursor.RemoveMemoryWatchpoint(&data);
			printf("...TRACE1 %llx...", ttdcursor.GetProgramCounter());
			data.addr = g_next_ret2;
			data.flags = TTD::BP_FLAGS::EXEC;
			data.size = 1;
			ttdcursor2.AddMemoryWatchpoint(&data);
			ttdcursor2.ReplayForward(&replayrez, last2, -1);
			ttdcursor2.RemoveMemoryWatchpoint(&data);
			printf("...TRACE2 %llx\n", ttdcursor.GetProgramCounter());
			step_size = DEFAULT_STEP_SIZE;
			goto deb;
		}
	}
}