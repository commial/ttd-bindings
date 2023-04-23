#include <windows.h> 
#include <stdio.h> 
#include <iostream>
#include <vector>
#include <cassert>
#include "TTD/TTD.hpp"

/*
@callback_value: value passed at callback registering
@addr_func: called function address
@addr_ret: return address after the call
*/
std::vector<TTD::GuestAddress> g_func_stack;
std::vector<TTD::GuestAddress> g_ret_stack;
std::vector<TTD::GuestAddress> g_func_stack2;
std::vector<TTD::GuestAddress> g_ret_stack2;

void callCallback_diff(unsigned __int64 callback_value, TTD::GuestAddress addr_func, TTD::GuestAddress addr_ret, struct TTD::TTD_Replay_IThreadView* arg4) {
	TTD::GuestAddress last_ret;
	if (addr_ret == 0) {
		// End of a call (RET's next instruction)
		// `addr_func` is the instruction address
		if (callback_value == 1) {
			if (!g_func_stack.empty() && !g_ret_stack.empty()) {
				g_func_stack.pop_back();
				last_ret = g_ret_stack.back();
				g_ret_stack.pop_back();
				assert(addr_func == last_ret);
			}
		}
		else if (callback_value == 2) {
			if (!g_func_stack2.empty() && !g_ret_stack2.empty()) {
				g_func_stack2.pop_back();
				last_ret = g_ret_stack2.back();
				g_ret_stack2.pop_back();
				assert(addr_func == last_ret);
			}
		}
		return;
	}

	if (callback_value == 1) {
		g_func_stack.push_back(addr_func);
		g_ret_stack.push_back(addr_ret);
	}
	else if (callback_value == 2) {
		g_func_stack2.push_back(addr_func);
		g_ret_stack2.push_back(addr_ret);
	}
}


struct Snapshot {

	std::vector<TTD::GuestAddress> func_stack;
	std::vector<TTD::GuestAddress> ret_stack;
	std::vector<TTD::GuestAddress> func_stack2;
	std::vector<TTD::GuestAddress> ret_stack2;
	TTD::Position cur;
	TTD::Position cur2;
};
struct Snapshot g_snapshot;

void takeSnapshot(TTD::Cursor* cursor, TTD::Cursor* cursor2) {
	g_snapshot.func_stack = g_func_stack; // This is a copy
	g_snapshot.ret_stack = g_ret_stack;
	g_snapshot.func_stack2 = g_func_stack2;
	g_snapshot.ret_stack2 = g_ret_stack2;
	memcpy(&g_snapshot.cur, cursor->GetPosition(), sizeof(TTD::Position));
	memcpy(&g_snapshot.cur2, cursor2->GetPosition(), sizeof(TTD::Position));
}

void restoreSnapshot(TTD::Cursor* cursor, TTD::Cursor* cursor2) {
	cursor->SetPosition(&g_snapshot.cur);
	cursor2->SetPosition(&g_snapshot.cur2);
	g_func_stack = g_snapshot.func_stack;
	g_ret_stack = g_snapshot.ret_stack;
	g_func_stack2 = g_snapshot.func_stack2;
	g_ret_stack2 = g_snapshot.ret_stack2;
}

const unsigned __int64 DEFAULT_STEP_SIZE = 10000;
const std::vector<TTD::GuestAddress> KNOWN_FUNC = {
	0x7ffb10ec1af0, // ntdll!RtlHeapAlloc
	0x7ffb10ebf320, // ntdll!RtlpAllocateHeapInternal
	0x7ffb10ebf2a0, // ntdll!RtlAllocateHeap
	0x7ffb10ebc320, // ntdll!RtlReAllocateHeap
	0x7ffb0b560eb0, // ESENT!JetSeek
	0x7ffb0b5621d0, // ESENT!JetRetrieveColumn
	0x7ffb0b9d2750, // ntdsai!dbReadDataColumns
	0x7ffb0b98be60, // ntdsai!SampGetMemberships
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

	ttdcursor.SetPosition(0x177B, 0x3F);
	ttdcursor2.SetPosition(0x6871, 0x3F);

	TTD::GuestAddress pc;
	TTD::GuestAddress pc2;
	TTD::TTD_Replay_ICursorView_ReplayResult replayrez;
deb:
	ttdcursor.SetCallReturnCallback((TTD::PROC_CallCallback) callCallback_diff, 1);
	ttdcursor2.SetCallReturnCallback((TTD::PROC_CallCallback) callCallback_diff, 2);
	takeSnapshot(&ttdcursor, &ttdcursor2);
	TTD::Position* cur = ttdcursor.GetPosition();
	TTD::Position* cur2 = ttdcursor2.GetPosition();

	while (step_size > 0) {
		printf("STEP %llu\n", step_size);
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

	printf("Last known sync reference: Trace 1: %llx:%llx\t| Trace 2: %llx:%llx\n", cur->Major, cur->Minor, cur2->Major, cur2->Minor);
	printf("Cur func: %llx -> %llx | %llx -> %llx\n", g_func_stack.back(), g_ret_stack.back(), g_func_stack2.back(), g_ret_stack2.back());
	
	// Look the call stacks for allow-listed functions
	TTD::GuestAddress ret_sync_addr = NULL;
	int stack_high;
	printf("Common func/ret:\n");
	for (stack_high = 0; stack_high < g_func_stack.size(); stack_high++) {
		// Check if the function is the same in both traces
		if (g_func_stack[stack_high] != g_func_stack2[stack_high]) {
			continue;
		}
		// Check the return address is the same
		if (g_ret_stack[stack_high] != g_ret_stack2[stack_high]) {
			continue;
		}
		for (int j = 0; j < stack_high; j++) {
			printf("\t");
		}
		printf("%llx -> %llx\n", g_func_stack[stack_high], g_ret_stack[stack_high]);
		if (std::find(KNOWN_FUNC.begin(), KNOWN_FUNC.end(), g_func_stack[stack_high]) != KNOWN_FUNC.end()) { // Function known for path diff, such as allocators
			ret_sync_addr = g_ret_stack[stack_high];
			break;
		}
	}
	
	if (ret_sync_addr != NULL) {
		printf("-> Resync traces after their RET on %llx", ret_sync_addr);

		// Create a BP on RET address
		TTD::TTD_Replay_MemoryWatchpointData data;
		data.addr = ret_sync_addr;
		data.flags = TTD::BP_FLAGS::EXEC;
		data.size = 1;
		ttdcursor.AddMemoryWatchpoint(&data);
		ttdcursor2.AddMemoryWatchpoint(&data);

		// Clean-up callbacks, we won't use them during the fast-forward
		ttdcursor.SetCallReturnCallback((TTD::PROC_CallCallback) NULL, 1);
		ttdcursor2.SetCallReturnCallback((TTD::PROC_CallCallback) NULL, 2);

		// Forward Trace 1
		ttdcursor.ReplayForward(&replayrez, last, -1);
		ttdcursor.RemoveMemoryWatchpoint(&data);
		printf("...TRACE1 %llx...", ttdcursor.GetProgramCounter());
		assert(ttdcursor.GetProgramCounter() == ret_sync_addr);

		// Forward Trace 2		
		ttdcursor2.ReplayForward(&replayrez, last2, -1);
		ttdcursor2.RemoveMemoryWatchpoint(&data);
		printf("...TRACE2 %llx\n", ttdcursor.GetProgramCounter());
		assert(ttdcursor2.GetProgramCounter() == ret_sync_addr);

		// Restore the step size
		step_size = DEFAULT_STEP_SIZE;

		// Remove no-more-used stacked elements
		for (auto j = g_func_stack.size(); j > stack_high; j--) {
			g_func_stack.pop_back();
			g_func_stack2.pop_back();
			g_ret_stack.pop_back();
			g_ret_stack2.pop_back();
		}
		assert(g_func_stack.size() == stack_high);

		// Let's do it all over again
		goto deb;
	}
}
