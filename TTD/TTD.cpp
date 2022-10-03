#include <windows.h> 
#include <stdio.h> 
#include <iostream>
#include "sha256.h"
#include "csts.h"
#include "utils.h"
#include "TTD.hpp"

namespace TTD {

	/**** Wrapping around the vftable ****/
	void Cursor::SetPosition(Position* position) {
		return this->cursor->ICursor->SetPosition(cursor, position);
	}
	void Cursor::SetPosition(unsigned __int64 Major, unsigned __int64 Minor) {
		Position pos;
		pos.Major = Major;
		pos.Minor = Minor;
		return this->SetPosition(&pos);
	}

	unsigned __int64 Cursor::GetThreadCount() {
		return this->cursor->ICursor->GetThreadCount(cursor);
	}

	GuestAddress Cursor::GetProgramCounter() {
		return this->GetProgramCounter(0);
	}
	GuestAddress  Cursor::GetProgramCounter(unsigned int ThreadId) {
		return this->cursor->ICursor->GetProgramCounter(cursor, ThreadId);
	}

	struct TTD_Replay_ActiveThreadInfo* Cursor::GetThreadList()
	{
		return this->cursor->ICursor->GetThreadList(cursor);
	}

	struct TTD_Replay_ThreadInfo* Cursor::GetThreadInfo() {
		return this->GetThreadInfo(0);
	}
	struct TTD_Replay_ThreadInfo* Cursor::GetThreadInfo(unsigned int ThreadId) {
		return this->cursor->ICursor->GetThreadInfo(cursor, ThreadId);
	}

	void* Cursor::GetCrossPlatformContext()
	{
		return this->GetCrossPlatformContext(0);
	}

	void* Cursor::GetCrossPlatformContext(uint32_t threadId) {
		void* ctxt = malloc(0xA70);
		return this->cursor->ICursor->GetCrossPlatformContext(cursor, ctxt, threadId);
	}

	struct MemoryBuffer* Cursor::QueryMemoryBuffer(GuestAddress address, unsigned __int64 size) {
		struct MemoryBuffer* memorybuffer = (struct MemoryBuffer*)malloc(sizeof(struct MemoryBuffer));
		struct TBuffer* buf = (struct TBuffer*)malloc(sizeof(struct TBuffer));
		if (buf == NULL)
			return NULL;
		buf->size = size;
		buf->dst_buffer = (void*)malloc(size);
		this->cursor->ICursor->QueryMemoryBuffer(cursor, memorybuffer, address, buf, 0);
		return memorybuffer;
	}

	struct TTD_Replay_ICursorView_ReplayResult* Cursor::ReplayForward(struct TTD_Replay_ICursorView_ReplayResult* replay_result_out, struct Position* posMax, unsigned __int64 stepCount) {
		return this->cursor->ICursor->ReplayForward(cursor, replay_result_out, posMax, stepCount);
	}

	struct TTD_Replay_ICursorView_ReplayResult* Cursor::ReplayBackward(struct TTD_Replay_ICursorView_ReplayResult* replay_result_out, struct Position* posMin, unsigned __int64 stepCount) {
		return this->cursor->ICursor->ReplayBackward(cursor, replay_result_out, posMin, stepCount);
	}

	void Cursor::SetCallReturnCallback(PROC_CallCallback callCallback, unsigned __int64 callback_value) {
		return this->cursor->ICursor->SetCallReturnCallback(cursor, callCallback, callback_value);
	}

	Position* Cursor::GetPosition(unsigned int ThreadId) {
		return this->cursor->ICursor->GetPosition(cursor, ThreadId);
	}

	Position* Cursor::GetPosition() {
		return this->GetPosition(0);
	}

	bool Cursor::AddMemoryWatchpoint(TTD_Replay_MemoryWatchpointData* data) {
		return this->cursor->ICursor->AddMemoryWatchpoint(cursor, data);
	}

	bool Cursor::RemoveMemoryWatchpoint(TTD_Replay_MemoryWatchpointData* data) {
		return this->cursor->ICursor->RemoveMemoryWatchpoint(cursor, data);
	}

	unsigned __int64 Cursor::GetModuleCount()
	{
		return this->cursor->ICursor->GetModuleCount(cursor);
	}
	struct TTD_Replay_ModuleInstance* Cursor::GetModuleList()
	{
		return this->cursor->ICursor->GetModuleList(cursor);
	}

	ReplayEngine::ReplayEngine(const wchar_t* ttdReplayPath, const wchar_t* ttdReplayCpuPath) {
		HINSTANCE hinstLib;
		PROC_Initiate InitiateReplayEngineHandshake;
		PROC_Create CreateReplayEngineWithHandshake;
		BYTE Source[48];
		char Destination[336];
		SHA256_CTX ctx;
		BYTE sha[32];

		hinstLib = LoadLibraryW(ttdReplayCpuPath);
		if (hinstLib == NULL) {
			throw std::exception("Unable to find TTDReplayCPU.dll");
		}

		hinstLib = LoadLibraryW(ttdReplayPath);
		if (hinstLib == NULL) {
			throw std::exception("Unable to find TTDReplay.dll");
		}
		InitiateReplayEngineHandshake = (PROC_Initiate)GetProcAddress(hinstLib, "InitiateReplayEngineHandshake");
		CreateReplayEngineWithHandshake = (PROC_Create)GetProcAddress(hinstLib, "CreateReplayEngineWithHandshake");

		int result = InitiateReplayEngineHandshake("DbgEng", Source);

		strncpy_s(Destination, (char*)Source, 0x2F);
		for (int i = 0; i < 2; ++i) {
			strncat_s(Destination, &aScopeOfLicense[0x66 * ((Source[i] - 48) % 0x11ui64)], 0x65ui64);
		}
		strncat_s(Destination, &aVYHVAX4gukZ8Wv[79 * ((Source[2] - 48i64) % 0xBui64)], 0x4Eui64);

		sha256_init(&ctx);
		sha256_update(&ctx, (unsigned char*)Destination, strlen(Destination));
		sha256_final(&ctx, sha);

		size_t sha_b64_size;
		char* sha_b64 = base64_encode(sha, 32, &sha_b64_size);
		char tmp[0x30];
		memset(tmp, 0, 0x30);
		memcpy(tmp, sha_b64, sha_b64_size);

		void* instance;
		result = CreateReplayEngineWithHandshake(tmp, &instance, version_guid);
		this->engine = (TTD_Replay_ReplayEngine*)instance;
	}

	/**** Wrapping around the vftable ****/
	bool ReplayEngine::Initialize(const wchar_t* trace_filename) {
		return this->engine->IReplayEngine->Initialize(engine, trace_filename);
	}

	struct TTD_SystemInfo* ReplayEngine::GetSystemInfo() {
		return this->engine->IReplayEngine->GetSystemInfo(engine);
	}

	unsigned __int64 ReplayEngine::GetThreadCount() {
		return this->engine->IReplayEngine->GetThreadCount(engine);
	}

	Position* ReplayEngine::GetFirstPosition() {
		return this->engine->IReplayEngine->GetFirstPosition(engine);
	}

	Position* ReplayEngine::GetLastPosition() {
		return this->engine->IReplayEngine->GetLastPosition(engine);
	}

	GuestAddress ReplayEngine::GetPebAddress() {
		return this->engine->IReplayEngine->GetPebAddress(engine);
	}

	Cursor ReplayEngine::NewCursor() {
		return Cursor(engine->IReplayEngine->NewCursor(engine, GUID_Cursor));
	}

	unsigned __int64 ReplayEngine::GetModuleCount() {
		return this->engine->IReplayEngine->GetModuleCount(engine);
	}

	const TTD_Replay_Module* ReplayEngine::GetModuleList() {
		return this->engine->IReplayEngine->GetModuleList(engine);
	}

	unsigned __int64  ReplayEngine::GetModuleLoadedEventCount()
	{
		return this->engine->IReplayEngine->GetModuleLoadedEventCount(engine);
	}

	const TTD_Replay_ModuleLoadedEvent* ReplayEngine::GetModuleLoadedEventList()
	{
		return this->engine->IReplayEngine->GetModuleLoadedEventList(engine);
	}

	const std::vector<TTD_Replay_ModuleLoadedEvent> ReplayEngine::GetModuleLoadedEvents()
	{
		return std::vector<TTD_Replay_ModuleLoadedEvent>(this->GetModuleLoadedEventList(), this->GetModuleLoadedEventList() + this->GetModuleLoadedEventCount());
	}

	unsigned __int64 ReplayEngine::GetModuleUnloadedEventCount()
	{
		return this->engine->IReplayEngine->GetModuleUnloadedEventCount(engine);
	}

	const TTD_Replay_ModuleUnloadedEvent* ReplayEngine::GetModuleUnloadedEventList()
	{
		return this->engine->IReplayEngine->GetModuleUnloadedEventList(engine);
	}

	const std::vector<TTD_Replay_ModuleUnloadedEvent> ReplayEngine::GetModuleUnloadedEvents()
	{
		return std::vector<TTD_Replay_ModuleUnloadedEvent>(this->GetModuleUnloadedEventList(), this->GetModuleUnloadedEventList() + this->GetModuleUnloadedEventCount());
	}

	unsigned __int64 ReplayEngine::GetThreadCreatedEventCount()
	{
		return this->engine->IReplayEngine->GetThreadCreatedEventCount(engine);
	}

	const TTD_Replay_ThreadCreatedEvent* ReplayEngine::GetThreadCreatedEventList()
	{
		return this->engine->IReplayEngine->GetThreadCreatedEventList(engine);
	}

	const std::vector<TTD_Replay_ThreadCreatedEvent> ReplayEngine::GetThreadCreatedEvents()
	{
		return std::vector<TTD_Replay_ThreadCreatedEvent>(this->GetThreadCreatedEventList(), this->GetThreadCreatedEventList() + this->GetThreadCreatedEventCount());
	}

	unsigned __int64 ReplayEngine::GetThreadTerminatedEventCount()
	{
		return this->engine->IReplayEngine->GetThreadTerminatedEventCount(engine);
	}

	const TTD_Replay_ThreadTerminatedEvent* ReplayEngine::GetThreadTerminatedEventList()
	{
		return this->engine->IReplayEngine->GetThreadTerminatedEventList(engine);
	}

	const std::vector<TTD_Replay_ThreadTerminatedEvent> ReplayEngine::GetThreadTerminatedEvents()
	{
		return std::vector<TTD_Replay_ThreadTerminatedEvent>(this->GetThreadTerminatedEventList(), this->GetThreadTerminatedEventList() + this->GetThreadTerminatedEventCount());
	}

	unsigned __int64 ReplayEngine::GetExceptionEventCount()
	{
		return this->engine->IReplayEngine->GetExceptionEventCount(engine);
	}

	const TTD_Replay_ExceptionEvent* ReplayEngine::GetExceptionEventList()
	{
		return this->engine->IReplayEngine->GetExceptionEventList(engine);
	}

	const std::vector<TTD_Replay_ExceptionEvent> ReplayEngine::GetExceptionEvents()
	{
		return std::vector<TTD_Replay_ExceptionEvent>(this->GetExceptionEventList(), this->GetExceptionEventList() + this->GetExceptionEventCount());
	}
}
