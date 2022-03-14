#include <windows.h> 
#include <stdio.h> 
#include <iostream>
#include <strsafe.h>
#include <dbghelp.h>
#include "utils.h"
#include "TTD.hpp"


/**** PDB ****/
#pragma comment(lib, "dbghelp.lib")

// Inspired from https://github.com/silverf0x/RpcView RpcView/Pdb.c

#define RSDS_SIGNATURE 'SDSR'
#define PDB_MAX_SYMBOL_SIZE	1000

//Only for PDB7.0 format!
typedef struct _CV_INFO_PDB70 {
	DWORD	CvSignature;
	GUID	Signature;
	DWORD	Age;
	BYTE	PdbFileName[MAX_PATH];
} CV_INFO_PDB70;

BOOL readMemory(void* dest, TTD::GuestAddress addr, unsigned __int64 size, TTD::Cursor* cursor) {
	BOOL result = TRUE;
	struct TTD::MemoryBuffer* memorybuffer = cursor->QueryMemoryBuffer(addr, size);
	if (memorybuffer->data == NULL or memorybuffer->size != size) {
		result = FALSE;
	}
	else {
		memcpy(dest, memorybuffer->data, size);
		free(memorybuffer->data);
	}
	free(memorybuffer);
	return TRUE;
}

BOOL WINAPI GetModulePdbInfo(TTD::Cursor *cursor, TTD::GuestAddress pModuleBase, CV_INFO_PDB70* pPdb70Info)
{
	IMAGE_DOS_HEADER		ImageDosHeader;
	IMAGE_NT_HEADERS		ImageNtHeaders;
	IMAGE_DEBUG_DIRECTORY	ImageDebugDirectory;
	BOOL					bResult = FALSE;

	// Assume 64 bits
	
	if (!readMemory(&ImageDosHeader, pModuleBase, sizeof(ImageDosHeader), cursor)) goto End;
	if (!readMemory(&ImageNtHeaders, pModuleBase + ImageDosHeader.e_lfanew, sizeof(ImageNtHeaders), cursor)) goto End;
	if (!readMemory(&ImageDebugDirectory, pModuleBase + ImageNtHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress, sizeof(ImageDebugDirectory), cursor)) goto End;
	if (!readMemory(pPdb70Info, pModuleBase + ImageDebugDirectory.AddressOfRawData, sizeof(*pPdb70Info), cursor)) goto End;
	if (pPdb70Info->CvSignature != RSDS_SIGNATURE)
	{
		printf("Invalid CvSignature");
		goto End;
	}
	bResult = TRUE;
End:
	return (bResult);
}

BOOL WINAPI GetPdbFilePath(TTD::Cursor* cursor, TTD::GuestAddress pModuleBase, char* PdbPath, size_t PdbPathSize)
{
	CV_INFO_PDB70	Pdb70Info;
	CHAR			SymbolPath[MAX_PATH] = { 0 };
	CHAR			NtSymbolPath[MAX_PATH] = { 0 };
	BOOL			bResult = FALSE;

	if (!GetModulePdbInfo(cursor, pModuleBase, &Pdb70Info)) goto End;
	if (strchr((char*)Pdb70Info.PdbFileName, '\\') != NULL) // Local Path
	{
		StringCbPrintfA((STRSAFE_LPSTR)PdbPath, PdbPathSize, "%hs", Pdb70Info.PdbFileName);
	}
	else
	{
		int iResult;
		char* pStar = NULL;
		if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", NtSymbolPath, sizeof(NtSymbolPath)) == 0) goto End;

		iResult = sscanf_s(NtSymbolPath, "srv*%259s", SymbolPath, sizeof(SymbolPath));
		if (iResult == 0) {
			iResult = sscanf_s(NtSymbolPath, "SRV*%259s", SymbolPath, sizeof(SymbolPath));
			if (iResult == 0) goto End;
		}
		pStar = strchr(SymbolPath + 4, '*');
		if (pStar != NULL) *pStar = 0;


		StringCbPrintfA((STRSAFE_LPSTR)PdbPath, PdbPathSize, "%s\\%s\\%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X%X\\%s",
			SymbolPath,
			Pdb70Info.PdbFileName,
			Pdb70Info.Signature.Data1,
			Pdb70Info.Signature.Data2,
			Pdb70Info.Signature.Data3,
			Pdb70Info.Signature.Data4[0],
			Pdb70Info.Signature.Data4[1],
			Pdb70Info.Signature.Data4[2],
			Pdb70Info.Signature.Data4[3],
			Pdb70Info.Signature.Data4[4],
			Pdb70Info.Signature.Data4[5],
			Pdb70Info.Signature.Data4[6],
			Pdb70Info.Signature.Data4[7],
			Pdb70Info.Age,
			Pdb70Info.PdbFileName
		);
	}
	bResult = TRUE;
End:
	return (bResult);
}
/**** END PDB ****/

 // Indent level. Can be negative if we start inside a function
__int64 g_cur_stack = 0;
// Current process, used for Symbol loading
HANDLE g_hProcess;

// Loaded module information
struct ModuleInfo {
	TTD::GuestAddress start_addr;
	TTD::GuestAddress end_addr;
	const wchar_t* name;
	struct ModuleInfo* next;
};
struct ModuleInfo* g_moduleinfo = NULL;

void callCallback_tree(unsigned __int64 callback_value, TTD::GuestAddress addr_func, TTD::GuestAddress addr_ret, struct TTD::TTD_Replay_IThreadView * thread_view) {
	// Indentation level printing
	if (addr_ret == 0) {
		g_cur_stack -= 1;
	}
	if (g_cur_stack < 0) {
		printf("\n**** PRE-START PARENT FUNCTION****\n");
		g_cur_stack = 0;
	}
	for (int i = 0; i < g_cur_stack; i++) {
		printf("| ");
	}
	if (addr_ret != 0) {
		g_cur_stack += 1;
	}

	// Find the current module
	struct ModuleInfo* ptr = g_moduleinfo;
	while (ptr != NULL) {
		if (addr_func >= ptr->start_addr && addr_func < ptr->end_addr)
			break;
		ptr = ptr->next;
	}
	if (ptr == NULL) {
		fprintf(stderr, "ERROR: %llx unknown module\n", addr_func);
		return;
	}

	TTD::Position* position = thread_view->IThreadView->GetPosition(thread_view);
	if (addr_ret == 0) {
		// Call's end, addr_func is the Call's next instruction
		printf("<- %ls!+%llx (%llx) [%llx:%llx] RETURN %llx\n", ptr->name, (addr_func - ptr->start_addr), addr_func, position->Major, position->Minor, thread_view->IThreadView->GetBasicReturnValue(thread_view));
	}
	else {
		// Actual Call

		// Try to resolve symbol
		// https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-symbol-information-by-address
		char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
		DWORD64 dwDisplacement = 0;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;
		if (SymFromAddr(g_hProcess, addr_func, &dwDisplacement, pSymbol))
		{
			// Symbol resolved, potential offset
			__int64 offset = addr_func-pSymbol->Address;
			if (offset != 0) {
				printf("-> %ls!%s+%llx", ptr->name, pSymbol->Name, offset);
			}
			else {
				printf("-> %ls!%s", ptr->name, pSymbol->Name);
			}
		}
		else
		{
			// Unresolved symbol
			printf("-> %ls!+%llx", ptr->name, (addr_func - ptr->start_addr));
		}
		// Additionnal info
		printf(" (%llx) [%llx:%llx]\n", addr_func, position->Major, position->Minor);
	}
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
/**** END ARGUMENT PARSING ****/

int main(int argc, char* argv[]) {
	TTD::ReplayEngine ttdengine = TTD::ReplayEngine();
	TTD::TTD_Replay_ICursorView_ReplayResult replayrez;
	int result;
	char pdb_path[PDB_MAX_SYMBOL_SIZE] = { 0 };

	if (argc <= 1) {
		printf("Usage:\n");
		printf("%s [options] trace\n", argv[0]);
		printf("Options:\n");
		printf("-b\tStarting position, as 1234:56\n");
		printf("-e\tEnding position, as 1234:56\n");
		return 0;
	}

	g_hProcess = GetCurrentProcess();
	if (!SymInitialize(g_hProcess, nullptr, false))
	{
		std::wcerr << "SymInitialize() failed\n";
		return -1;
	}

	std::cout << "Openning the trace\n";
	wchar_t trace_path[MAX_PATH] = { 0 };
	StringCbPrintfW(trace_path, MAX_PATH, L"%S", argv[argc - 1]);
	result = ttdengine.Initialize(trace_path);

	if (result == 0) {
		std::wcerr << "Fail to open the trace";
		exit(-1);
	}

	// Cursor needed before symbol resolution, to fetch PDB information from trace memory
	TTD::Cursor ttdcursor = ttdengine.NewCursor();
	TTD::Position* first = ttdengine.GetFirstPosition();
	TTD::Position* last = ttdengine.GetLastPosition();
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

	std::cout << "ModuleList:\n";
	TTD::TTD_Replay_Module* mod_list = ttdengine.GetModuleList();
	for (int i = 0; i < ttdengine.GetModuleCount(); i++) {
		printf("%llx\t%llx\t%ls\n", mod_list[i].base_addr, mod_list[i].imageSize, mod_list[i].path);

		// Extract filename
		std::wstring fname = mod_list[i].path;
		fname = fname.substr(fname.find_last_of(L"\\"));
		fname = fname.substr(1, fname.find_last_of(L".") - 1);

		// Add to g_moduleinfo list
		struct ModuleInfo* modin = (struct ModuleInfo*)malloc(sizeof(struct ModuleInfo));
		modin->next = g_moduleinfo;
		modin->name = _wcsdup(fname.c_str());
		modin->start_addr = mod_list[i].base_addr;
		modin->end_addr = mod_list[i].base_addr + mod_list[i].imageSize;
		g_moduleinfo = modin;
	}

	std::cout << "Load symbols:\n";
	for (int i = 0; i < ttdengine.GetModuleCount(); i++) {
		printf("%ls", mod_list[i].path);

		if (!GetPdbFilePath(&ttdcursor, mod_list[i].base_addr, pdb_path, PDB_MAX_SYMBOL_SIZE)) {
			/*
			 * If PDB is not available, fallback on the DLL (if available)
			 * /!\ might not be the same than the one in the trace!
			 */
			printf(" ... fallback to DLL ... ");
			StringCbPrintfA((STRSAFE_LPSTR)pdb_path, PDB_MAX_SYMBOL_SIZE, "%S", mod_list[i].path);
		}
		if (SymLoadModuleEx(g_hProcess, NULL, pdb_path,	NULL, mod_list[i].base_addr, mod_list[i].imageSize,	NULL, 0))
			printf(" OK (%s)\n", pdb_path);
		else
			printf(" SymLoadModuleEx(%s) returned error : %d\n", pdb_path, GetLastError());

	}

	// Set the callback
	ttdcursor.SetCallReturnCallback((TTD::PROC_CallCallback) callCallback_tree, 0);

	ttdcursor.ReplayForward(&replayrez, &end, -1);
	return 0;
}