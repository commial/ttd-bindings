#pragma once

namespace TTD {

	typedef struct TTD_Replay_ReplayEngine TTD_Replay_ReplayEngine;

	typedef struct Position {
		unsigned __int64 Major; // Sequence
		unsigned __int64 Minor; // Steps
	} Position;


	// Size: 0xA70
	struct TTD_Replay_RegisterContext {
		unsigned __int64 unk1;
		unsigned __int64 unk2;
		unsigned __int64 unk3;
		unsigned __int64 unk4;
		unsigned __int64 unk5;
		unsigned __int64 unk6;
		unsigned __int64 unk7;
		USHORT cs;
		USHORT ds;
		USHORT es;
		USHORT fs;
		USHORT gs;
		USHORT ss;
		unsigned __int32 efl;
		unsigned __int64 unk8;
		unsigned __int64 unk9;
		unsigned __int64 unk10;
		unsigned __int64 unk11;
		unsigned __int64 unk12;
		unsigned __int64 unk13;
		unsigned __int64 rax;
		unsigned __int64 rcx;
		unsigned __int64 rdx;
		unsigned __int64 rbx;
		unsigned __int64 rsp;
		unsigned __int64 rbp;
		unsigned __int64 rsi;
		unsigned __int64 rdi;
		unsigned __int64 r8;
		unsigned __int64 r9;
		unsigned __int64 r10;
		unsigned __int64 r11;
		unsigned __int64 r12;
		unsigned __int64 r13;
		unsigned __int64 r14;
		unsigned __int64 r15;
		unsigned __int64 rip;
		USHORT fpcw;
	};

	// Size: 0x2280 (1104 64bits)
	struct TTD_Replay_ExtendedRegisterContext {

	};

	struct TTD_Replay_ThreadInfo {
		unsigned __int32 unk1; // likely the thread index (2, 3, 9, ...)
		unsigned __int32 threadid;
	};

	typedef unsigned __int64 GuestAddress;

	typedef struct TTD_Replay_ICursor TTD_Replay_ICursor;

	struct MemoryBuffer {
		GuestAddress addr;
		void* data;
		unsigned __int64 size;
	};

	struct TBuffer {
		void* dst_buffer;
		unsigned __int64 size;
	};

	typedef struct TTD_Replay_ICursorView_ReplayResult {
		unsigned __int64 unk1;
		// Number of step made
		unsigned __int64 stepCount;
		unsigned __int64 unk3;
		unsigned __int64 unk4;
		unsigned __int64 unk5;
		unsigned __int64 unk6;
	} TTD_Replay_ICursorView_ReplayResult;

	typedef struct TTD_Replay_IThreadView {
		struct TTD_Replay_IThreadView_vftable* IThreadView;
	} TTD_Replay_IThreadView;

	typedef struct TTD_Replay_MemoryWatchpointData {
		GuestAddress addr;
		unsigned __int64 size;
		unsigned __int64 flags;
	} TTD_Replay_MemoryWatchpointData;

	const enum BP_FLAGS {
		WRITE = 2,
		READ = 3,
		EXEC = 4
	};

	typedef struct TTD_Replay_ActiveThreadInfo {
		TTD_Replay_ThreadInfo* info;
		// next Major:Minor position where this thread is active
		uint64_t nextMajor;
		uint64_t nextMinor;
		// last Major:Minor position where this thread was active
		uint64_t lastMajor;
		uint64_t lastMinor;
	}TTD_Replay_ActiveThreadInfo;
	/*
	*(_QWORD *)this = &TTD::Replay::Cursor::`vftable'{for `TTD::Replay::ICursor'};
	  *((_QWORD *)this + 1) = &TTD::Replay::Cursor::`vftable'{for `TTD::Replay::ICursorInternals'};
	  *((_QWORD *)this + 2) = &TTD::Replay::Cursor::`vftable'{for `ICursor_v1::ICursor'};
	  *((_QWORD *)this + 3) = &TTD::Replay::Cursor::`vftable'{for `ICursor_v2::ICursor'};
	*/

	typedef struct TTD_Replay_ICursor_vftable {
		//  void *___7Cursor_Replay_TTD__6BICursor_12_@;
		void* unk1;
		struct MemoryBuffer* (*QueryMemoryBuffer)(TTD_Replay_ICursor* self, struct MemoryBuffer* memorybuffer_out, GuestAddress, struct TBuffer* buf, unsigned int queryMemoryPolicy);
		//  void *_QueryMemoryBufferWithRanges_Cursor_Replay_TTD__UEBA_AUMemoryBufferWithRanges_23_W4GuestAddress_Nirvana__U_$TBufferView_$0A__3__KPEAUMemoryRange_23_W4QueryMemoryPolicy_23__Z;
		void* unk3;
		//  void (__stdcall __high *_SetDefaultMemoryPolicy_Cursor_Replay_TTD__UEAAXW4QueryMemoryPolicy_23__Z)(enum TTD::Replay::QueryMemoryPolicy);
		void* unk4;
		//  enum TTD::Replay::QueryMemoryPolicy (__high *_GetDefaultMemoryPolicy_Cursor_Replay_TTD__UEBA_AW4QueryMemoryPolicy_23_XZ)(void);
		void* unk5;
		//  void *(__fastcall *_UnsafeGetReplayEngine_Cursor_Replay_TTD__UEBAPEAXAEBU_GUID___Z)(TTD::Replay::Cursor *__hidden this, const struct _GUID *);
		void* unk6;
		//  const void *(__fastcall *_UnsafeAsInterface_Cursor_Replay_TTD__UEBAPEBXAEBU_GUID___Z)(TTD::Replay::Cursor *__hidden this, const struct _GUID *);
		void* unk7;
		//  void *(__fastcall *_UnsafeAsInterface_Cursor_Replay_TTD__UEAAPEAXAEBU_GUID___Z)(TTD::Replay::Cursor *__hidden this, const struct _GUID *);
		void* unk8;
		struct TTD_Replay_ThreadInfo* (__stdcall* GetThreadInfo)(TTD_Replay_ICursor* self, unsigned int ThreadId);
		//  __int64 (__fastcall *_GetTebAddress_Cursor_Replay_TTD__UEBA_AW4GuestAddress_Nirvana__W4ThreadId_3__Z)(__int64);
		void* unk10;
		struct Position* (__fastcall *GetPosition)(TTD_Replay_ICursor* self, unsigned int ThreadId);
		//  const struct Position *(__stdcall __high *_GetPreviousPosition_Cursor_Replay_TTD__UEBAAEBUPosition_23_W4ThreadId_3__Z)(enum TTD::ThreadId);
		void* unk12;
		GuestAddress(__stdcall* GetProgramCounter)(TTD_Replay_ICursor* self, unsigned int ThreadId);
		//  enum Nirvana::GuestAddress (__stdcall __high *_GetStackPointer_Cursor_Replay_TTD__UEBA_AW4GuestAddress_Nirvana__W4ThreadId_3__Z)(enum TTD::ThreadId);
		void* unk14;
		//  enum Nirvana::GuestAddress (__stdcall __high *_GetFramePointer_Cursor_Replay_TTD__UEBA_AW4GuestAddress_Nirvana__W4ThreadId_3__Z)(enum TTD::ThreadId);
		void* unk15;
		//  unsigned __int64 (__stdcall __high *_GetBasicReturnValue_Cursor_Replay_TTD__UEBA_KW4ThreadId_3__Z)(enum TTD::ThreadId);
		void* unk16;
		struct TTD_Replay_RegisterContext* (__stdcall* GetCrossPlatformContext)(TTD_Replay_ICursor* self, struct TTD_Replay_RegisterContext* out, uint32_t threadId);
		struct TTD_Replay_ExtendedRegisterContext* (__stdcall* GetAvxExtendedContext)(TTD_Replay_ICursor* self, struct TTD_Replay_ExtendedRegisterContext* out);
		//  unsigned __int64 (__fastcall *_GetModuleCount_Cursor_Replay_TTD__UEBA_KXZ)(TTD::Replay::Cursor *__hidden this);
		unsigned __int64(__fastcall* GetModuleCount)(TTD_Replay_ICursor* self);
		//  const struct TTD::Replay::ModuleInstance *(__fastcall *_GetModuleList_Cursor_Replay_TTD__UEBAPEBUModuleInstance_23_XZ)(TTD::Replay::Cursor *__hidden this);
		struct TTD_Replay_ModuleInstance* (__fastcall* GetModuleList)(TTD_Replay_ICursor* self);
		unsigned __int64(__fastcall* GetThreadCount)(TTD_Replay_ICursor* self);
		struct TTD_Replay_ActiveThreadInfo* (__fastcall* GetThreadList)(TTD_Replay_ICursor* self);
		void(__fastcall* SetEventMask)(TTD_Replay_ICursor* self, DWORD eventMask);
		//  enum EventMask (__fastcall *_GetEventMask_Cursor_Replay_TTD__UEBA_AW4EventMask_23_XZ)(TTD::Replay::Cursor *__hidden this);
		void* unk24;
		//  void (__stdcall __high *_SetGapKindMask_Cursor_Replay_TTD__UEAAXW4GapKindMask_23__Z)(enum TTD::Replay::GapKindMask);
		void* unk25;
		//  enum TTD::Replay::GapKindMask (__high *_GetGapKindMask_Cursor_Replay_TTD__UEBA_AW4GapKindMask_23_XZ)(void);
		void* unk26;
		//  void (__stdcall __high *_SetGapEventMask_Cursor_Replay_TTD__UEAAXW4GapEventMask_23__Z)(enum TTD::Replay::GapEventMask);
		void* unk27;
		//  enum TTD::Replay::GapEventMask (__high *_GetGapEventMask_Cursor_Replay_TTD__UEBA_AW4GapEventMask_23_XZ)(void);
		void* unk28;
		//  void (__stdcall __high *_SetExceptionMask_Cursor_Replay_TTD__UEAAXW4ExceptionMask_23__Z)(enum TTD::Replay::ExceptionMask);
		void* unk29;
		//  enum TTD::Replay::ExceptionMask (__high *_GetExceptionMask_Cursor_Replay_TTD__UEBA_AW4ExceptionMask_23_XZ)(void);
		void* unk30;
		//  void (__stdcall __high *_SetReplayFlags_Cursor_Replay_TTD__UEAAXW4ReplayFlags_23__Z)(enum TTD::Replay::ReplayFlags);
		void* unk31;
		//  enum TTD::Replay::ReplayFlags (__high *_GetReplayFlags_Cursor_Replay_TTD__UEBA_AW4ReplayFlags_23_XZ)(void);
		void* unk32;
		bool (__fastcall *AddMemoryWatchpoint)(TTD_Replay_ICursor* self, struct TTD_Replay_MemoryWatchpointData *data);
		bool (__fastcall *RemoveMemoryWatchpoint)(TTD_Replay_ICursor* self, struct TTD_Replay_MemoryWatchpointData* data);
		//  bool (*_AddPositionWatchpoint_Cursor_Replay_TTD__UEAA_NAEBUPositionWatchpointData_23__Z)(TTD::Replay::Cursor *__hidden this, const struct TTD::Replay::PositionWatchpointData *);
		void* unk35;
		//  bool (*_RemovePositionWatchpoint_Cursor_Replay_TTD__UEAA_NAEBUPositionWatchpointData_23__Z)(TTD::Replay::Cursor *__hidden this, const struct TTD::Replay::PositionWatchpointData *);
		void* unk36;
		//  void (__fastcall *_Clear_Cursor_Replay_TTD__UEAAXXZ)(TTD::Replay::Cursor *__hidden this);
		void* unk37;
		void(__fastcall* SetPosition)(TTD_Replay_ICursor* self, struct Position*);
		//  void (__stdcall __high *_SetPositionOnThread_Cursor_Replay_TTD__UEAAXW4UniqueThreadId_23_AEBUPosition_23__Z)(enum TTD::Replay::UniqueThreadId, const struct Position *);
		void* unk39;
		//  void (__fastcall *_SetMemoryWatchpointCallback_Cursor_Replay_TTD__UEAAXQ6A_N_KAEBUMemoryWatchpointResult_ICursorView_23_PEBVIThreadView_23__Z_K_Z)(TTD::Replay::Cursor *__hidden this, bool (__stdcall *const)(unsigned __int64, const struct TTD::Replay::ICursorView::MemoryWatchpointResult *, const struct TTD::Replay::IThreadView *), unsigned __int64);
		void* unk40;
		//  void (__fastcall *_SetPositionWatchpointCallback_Cursor_Replay_TTD__UEAAXQ6A_N_KAEBUPosition_23_PEBVIThreadView_23__Z_K_Z)(TTD::Replay::Cursor *__hidden this, bool (__stdcall *const)(unsigned __int64, const struct Position *, const struct TTD::Replay::IThreadView *), unsigned __int64);
		void* unk41;
		//  void (__fastcall *_SetGapEventCallback_Cursor_Replay_TTD__UEAAXQ6A_N_KW4GapKind_23_W4GapEventType_23_PEBVIThreadView_23__Z_K_Z)(TTD::Replay::Cursor *__hidden this, bool (__stdcall __high *const)(unsigned __int64, enum TTD::Replay::GapKind, enum TTD::Replay::GapEventType, const struct TTD::Replay::IThreadView *), unsigned __int64);
		void* unk42;
		//  void (__fastcall *_SetThreadContinuityBreakCallback_Cursor_Replay_TTD__UEAAXQ6AX_K_Z_K_Z)(TTD::Replay::Cursor *__hidden this, void (__stdcall *const)(unsigned __int64), unsigned __int64);
		void* unk43;
		//  void (__fastcall *_SetReplayProgressCallback_Cursor_Replay_TTD__UEAAXQ6AX_KAEBUPosition_23__Z_K_Z)(TTD::Replay::Cursor *__hidden this, void (__stdcall *const)(unsigned __int64, const struct Position *), unsigned __int64);
		void* unk44;
		//  void (__fastcall *_SetFallbackCallback_Cursor_Replay_TTD__UEAAXQ6AX_K_NW4GuestAddress_Nirvana__0PEBVIThreadView_23__Z_K_Z)(TTD::Replay::Cursor *__hidden this, void (__stdcall __high *const)(unsigned __int64, bool, enum Nirvana::GuestAddress, unsigned __int64, const struct TTD::Replay::IThreadView *), unsigned __int64);
		void* unk45;
		void(__fastcall* SetCallReturnCallback)(TTD_Replay_ICursor* self, void(__stdcall* const)(unsigned __int64 callback_value, GuestAddress addr_func, GuestAddress addr_ret, struct TTD_Replay_IThreadView* thread_info), unsigned __int64 callback_value);
		//  void (__fastcall *_SetIndirectJumpCallback_Cursor_Replay_TTD__UEAAXQ6AX_KW4GuestAddress_Nirvana__PEBVIThreadView_23__Z_K_Z)(TTD::Replay::Cursor *__hidden this, void (__stdcall __high *const)(unsigned __int64, enum Nirvana::GuestAddress, const struct TTD::Replay::IThreadView *), unsigned __int64);
		void* unk47;
		//  void (__fastcall *_SetRegisterChangedCallback_Cursor_Replay_TTD__UEAAXQ6AX_KEPEBX10PEBVIThreadView_23__Z_K_Z)(TTD::Replay::Cursor *__hidden this, void (__stdcall *const)(unsigned __int64, unsigned __int8, const void *, const void *, unsigned __int64, const struct TTD::Replay::IThreadView *), unsigned __int64);
		void* unk48;
		struct TTD_Replay_ICursorView_ReplayResult* (__stdcall* ReplayForward)(TTD_Replay_ICursor* self, struct TTD_Replay_ICursorView_ReplayResult* replay_result_out, struct Position* posMax, unsigned __int64 stepCount);
		struct TTD_Replay_ICursorView_ReplayResult* (__stdcall* ReplayBackward)(TTD_Replay_ICursor* self, struct TTD_Replay_ICursorView_ReplayResult* replay_result_out, struct Position* posMin, unsigned __int64 stepCount);
		void(__fastcall* InterruptReplay)(TTD_Replay_ICursor* self);
		//  const struct TTD::Replay::IEngineInternals *(__fastcall *_GetInternals_ReplayEngine_Replay_TTD__UEBAPEBVIEngineInternals_23_XZ)(TTD::Replay::ReplayEngine *__hidden this);
		void* unk52;
		//  const struct TTD::Replay::IEngineInternals *(__fastcall *anonymous_0)(TTD::Replay::ReplayEngine *__hidden this);
		void* unk53;
		//  unsigned __int64 (__stdcall __high *_GetInternalData_Cursor_Replay_TTD__UEBA_KW4InternalDataId_ICursorView_23_PEAX_K_Z)(enum TTD::Replay::ICursorView::InternalDataId, void *, unsigned __int64);
		void* unk54;
		//  void *(__fastcall *___ECursor_Replay_TTD__UEAAPEAXI_Z)(TTD::Replay::Cursor *__hidden this, unsigned int);
		void* unk55;
		//  void (__fastcall *_Destroy_Cursor_Replay_TTD__UEAAXXZ)(TTD::Replay::Cursor *__hidden this);
		void* unk56;
	} TTD_Replay_ICursor_vftable;

	// Size 0x1018
	typedef struct  TTD_Replay_ICursor {
		TTD_Replay_ICursor_vftable* ICursor;
	} TTD_Replay_ICursor;

	typedef struct TTD_SystemInfo {

	} TTD_SystemInfo;

	typedef struct TTD_Replay_Module {
		WCHAR* path;
		size_t path_len;
		GuestAddress base_addr;
		size_t imageSize;
		DWORD checkSum;
		short timestampEnd;
	} TTD_Replay_Module;

	typedef struct TTD_Replay_ModuleInstance {
		TTD_Replay_Module* module;
		unsigned __int64 number;
		unsigned __int64 unk; // Always equal to -2
	} TTD_Replay_ModuleInstance;

	/*
	* TTD::Replay::ReplayEngine *__fastcall TTD::Replay::ReplayEngine::ReplayEngine(TTD::Replay::ReplayEngine *this)
	*
	*
	  *(_QWORD *)this = &TTD::Replay::ReplayEngine::`vftable'{for `TTD::Replay::IReplayEngine'};
	  *((_QWORD *)this + 1) = &TTD::Replay::ReplayEngine::`vftable'{for `TTD::Replay::IEngineInternals'};
	  *((_QWORD *)this + 2) = &TTD::Replay::ReplayEngine::`vftable'{for `IReplayEngine_v1::IReplayEngine'};
	  *((_QWORD *)this + 3) = &TTD::Replay::ReplayEngine::`vftable'{for `IReplayEngine_v2::IReplayEngine'};
	  *((_QWORD *)this + 4) = &TTD::Replay::ReplayEngine::`vftable'{for `IReplayEngine_v3::IReplayEngine'};
	  *((_QWORD *)this + 5) = &TTD::Replay::ReplayEngine::`vftable'{for `IReplayEngine_v4::IReplayEngine'};
	*/

	typedef struct TTD_Replay_IReplayEngine_vftable {
		//const void* (__fastcall* ___7ReplayEngine_Replay_TTD__6BIReplayEngine_12_@)(TTD::Replay::ReplayEngine* __hidden this, const struct _GUID*);
		void* unk1;
		//void* (__fastcall* _UnsafeAsInterface_ReplayEngine_Replay_TTD__UEAAPEAXAEBU_GUID___Z)(TTD::Replay::ReplayEngine* __hidden this, const struct _GUID*);
		void* unk2;
		GuestAddress(*GetPebAddress)(TTD_Replay_ReplayEngine* self);
		struct TTD_SystemInfo* (__fastcall* GetSystemInfo)(TTD_Replay_ReplayEngine* self);
		struct Position* (__fastcall* GetFirstPosition)(TTD_Replay_ReplayEngine* self);
		struct Position* (__fastcall* GetLastPosition)(TTD_Replay_ReplayEngine* self);
		//const struct Position* (__fastcall* anonymous_0)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk7;
		//	enum RecordingType(__fastcall* _GetRecordingType_ReplayEngine_Replay_TTD__UEBA_AW4RecordingType_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk8;
		//	const struct TTD::Replay::ThreadInfo* (__stdcall __high* _GetThreadInfo_ReplayEngine_Replay_TTD__UEBAAEBUThreadInfo_23_W4UniqueThreadId_23__Z)(enum TTD::Replay::UniqueThreadId);
		void* unk9;
		unsigned __int64(__fastcall* GetThreadCount)(TTD_Replay_ReplayEngine* self);
		//	const struct TTD::Replay::ThreadInfo* (__fastcall* _GetThreadList_ReplayEngine_Replay_TTD__UEBAPEBUThreadInfo_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk11;
		//	const unsigned __int64* (__fastcall* _GetThreadFirstPositionIndex_ReplayEngine_Replay_TTD__UEBAPEB_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk12;
		//	const unsigned __int64* (__fastcall* _GetThreadLastPositionIndex_ReplayEngine_Replay_TTD__UEBAPEB_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk13;
		//	const unsigned __int64* (__fastcall* _GetThreadLifetimeFirstPositionIndex_ReplayEngine_Replay_TTD__UEBAPEB_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk14;
		//	const unsigned __int64* (__fastcall* _GetThreadLifetimeLastPositionIndex_ReplayEngine_Replay_TTD__UEBAPEB_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk15;
		//	unsigned __int64(__fastcall* _GetThreadCreatedEventCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk16;
		//	const struct TTD::Replay::ThreadCreatedEvent* (__fastcall* _GetThreadCreatedEventList_ReplayEngine_Replay_TTD__UEBAPEBUThreadCreatedEvent_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk17;
		//	unsigned __int64(__fastcall* _GetThreadTerminatedEventCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk18;
		//	const struct TTD::Replay::ThreadTerminatedEvent* (__fastcall* _GetThreadTerminatedEventList_ReplayEngine_Replay_TTD__UEBAPEBUThreadTerminatedEvent_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk19;
		unsigned __int64(__fastcall* GetModuleCount)(TTD_Replay_ReplayEngine* self);
		struct TTD_Replay_Module* (__fastcall* GetModuleList)(TTD_Replay_ReplayEngine* self);
		//	unsigned __int64(__fastcall* _GetModuleInstanceCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk22;
		//	const struct TTD::Replay::ModuleInstance* (__fastcall* _GetModuleInstanceList_ReplayEngine_Replay_TTD__UEBAPEBUModuleInstance_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk23;
		//	const unsigned __int64* (__fastcall* _GetModuleInstanceUnloadIndex_ReplayEngine_Replay_TTD__UEBAPEB_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk24;
		//	unsigned __int64(__fastcall* _GetModuleLoadedEventCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk25;
		//	const struct TTD::Replay::ModuleLoadedEvent* (__fastcall* _GetModuleLoadedEventList_ReplayEngine_Replay_TTD__UEBAPEBUModuleLoadedEvent_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk26;
		//	unsigned __int64(__fastcall* _GetModuleUnloadedEventCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk27;
		//	const struct TTD::Replay::ModuleUnloadedEvent* (__fastcall* _GetModuleUnloadedEventList_ReplayEngine_Replay_TTD__UEBAPEBUModuleUnloadedEvent_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk28;
		//	unsigned __int64(__fastcall* _GetExceptionEventCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk29;
		//	const struct TTD::Replay::ExceptionEvent* (__fastcall* _GetExceptionEventList_ReplayEngine_Replay_TTD__UEBAPEBUExceptionEvent_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk30;
		//	const struct TTD::Replay::ExceptionEvent* (__fastcall* _GetExceptionAtOrAfterPosition_ReplayEngine_Replay_TTD__UEBAPEBUExceptionEvent_23_AEBUPosition_23__Z)(TTD::Replay::ReplayEngine* __hidden this, const struct Position*);
		void* unk31;
		//	unsigned __int64(__fastcall* _GetKeyframeCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk32;
		//	const struct Position* (__fastcall* _GetKeyframeList_ReplayEngine_Replay_TTD__UEBAPEBUPosition_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk33;
		//	unsigned __int64(__fastcall* _GetRecordClientCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk34;
		//	const struct TTD::Replay::RecordClient* (__fastcall* _GetRecordClientList_ReplayEngine_Replay_TTD__UEBAPEBURecordClient_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk35;
		//	const struct TTD::Replay::RecordClient* (__stdcall __high* _GetRecordClient_ReplayEngine_Replay_TTD__UEBAAEBURecordClient_23_W4RecordClientId_3__Z)(enum TTD::RecordClientId);
		void* unk36;
		//	unsigned __int64(__fastcall* _GetCustomEventCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk37;
		//	const struct TTD::Replay::CustomEvent* (__fastcall* _GetCustomEventList_ReplayEngine_Replay_TTD__UEBAPEBUCustomEvent_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk38;
		//	unsigned __int64(__fastcall* _GetActivityCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk39;
		//	const struct TTD::Replay::Activity* (__fastcall* _GetActivityList_ReplayEngine_Replay_TTD__UEBAPEBUActivity_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk40;
		//	unsigned __int64(__fastcall* _GetIslandCount_ReplayEngine_Replay_TTD__UEBA_KXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk41;
		//	const struct TTD::Replay::Island* (__fastcall* _GetIslandList_ReplayEngine_Replay_TTD__UEBAPEBUIsland_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk42;
		struct TTD_Replay_ICursor* (__fastcall* NewCursor)(TTD_Replay_ReplayEngine* self, const unsigned char* guid);
		//	enum TTD::Replay::IndexStatus(__stdcall __high* _BuildIndex_ReplayEngine_Replay_TTD__UEAA_AW4IndexStatus_23_P6AXPEBXPEBUIndexBuildProgressType_23__Z0W4IndexBuildFlags_23__Z)(void(__stdcall __high*)(const void*, const struct TTD::Replay::IndexBuildProgressType*), const void*, enum TTD::Replay::IndexBuildFlags);
		void* unk44;
		//	enum TTD::Replay::IndexStatus(__high* _GetIndexStatus_ReplayEngine_Replay_TTD__UEBA_AW4IndexStatus_23_XZ)(void);
		void* unk45;
		//	struct TTD::Replay::IndexFileStats(__high* _GetIndexFileStats_ReplayEngine_Replay_TTD__UEBA_AUIndexFileStats_23_XZ)(void);
		void* unk46;
		//	__int64(__fastcall* _RegisterDebugModeAndLogging_ReplayEngine_Replay_TTD__UEAAXW4DebugModeType_23_PEAVErrorReporting_3__Z)(__int64, int, __int64);
		void* unk47;
		//	const struct TTD::Replay::IEngineInternals* (__fastcall* _GetInternals_ReplayEngine_Replay_TTD__UEBAPEBVIEngineInternals_23_XZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk48;
		//	const struct TTD::Replay::IEngineInternals* (__fastcall* anonymous_1)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk49;
		//	void* (__fastcall* ___EReplayEngine_Replay_TTD__UEAAPEAXI_Z)(TTD::Replay::ReplayEngine* __hidden this, unsigned int);
		void* unk50;
		//	void(__fastcall* _Destroy_ReplayEngine_Replay_TTD__UEAAXXZ)(TTD::Replay::ReplayEngine* __hidden this);
		void* unk51;
		bool(__fastcall* Initialize)(TTD_Replay_ReplayEngine* self, const wchar_t* trace_filename);
	} TTD_Replay_IReplayEngine_vftable;

	typedef struct TTD_Replay_ReplayEngine {
		TTD_Replay_IReplayEngine_vftable* IReplayEngine;
	} TTD_Replay_ReplayEngine;

	typedef struct TTD_Replay_IThreadView_vftable {
		// TTD::Replay::ExecutionState::GetThreadInfo(void)
		void* unk1;
		// offset TTD::Replay::ExecutionState::GetTebAddress(void)
		void* unk2;
		Position* (*GetPosition)(TTD_Replay_IThreadView* self);
		// TTD::Replay::ExecutionState::GetPreviousPosition(void)
		void* unk4;
		GuestAddress (*GetProgramCounter)(TTD_Replay_IThreadView* self);
		// TTD::Replay::ExecutionState::GetStackPointer(void)
		void* unk6;
		// TTD::Replay::ExecutionState::GetFramePointer(void)
		void* unk7;
		unsigned __int64 (*GetBasicReturnValue)(TTD_Replay_IThreadView* self);
		// TTD::Replay::ExecutionState::GetCrossPlatformContext(void)
		void* unk9;
		// TTD::Replay::ExecutionState::GetAvxExtendedContext(void)
		void* unk10;
		// TTD::Replay::ExecutionState::QueryMemoryRange(Nirvana::GuestAddress)
		void* unk11;
		// TTD::Replay::ExecutionState::QueryMemoryBuffer(Nirvana::GuestAddress, TTD::TBufferView<0>)
		void* unk12;
		// TTD::Replay::ExecutionState::QueryMemoryBufferWithRanges(Nirvana::GuestAddress, TTD::TBufferView<0>, unsigned __int64, TTD::Replay::MemoryRange*)
		void* unk13;
		// Destructor
		void* unk14;
	} TTD_Replay_IThreadView_vftable;


	typedef unsigned int(__cdecl* PROC_Initiate)(const char* seed, BYTE* b64rand_out);
	typedef unsigned int(__cdecl* PROC_Create)(const char* handshake, void* ReplayEngine_out, BYTE* guid_version);
	typedef void(__stdcall* const PROC_CallCallback)(unsigned __int64 callback_value, GuestAddress addr_func, GuestAddress addr_ret, struct TTD_Replay_IThreadView* thread_info);

	class Cursor {

	private:
		TTD_Replay_ICursor* cursor;
	public:
		explicit Cursor(TTD_Replay_ICursor* input) { cursor = input; }
		Cursor(const Cursor&) = default;
		Cursor& operator=(const Cursor&) = default;

		/**** Wrapping around the vftable ****/
		void SetPosition(Position* position);
		void SetPosition(unsigned __int64 Major, unsigned __int64 Minor);
		struct Position* GetPosition();
		struct Position* GetPosition(unsigned int ThreadId);
		unsigned __int64 GetThreadCount();
		GuestAddress GetProgramCounter();
		GuestAddress GetProgramCounter(unsigned int ThreadId);
		struct TTD_Replay_ThreadInfo* GetThreadInfo();
		struct TTD_Replay_ActiveThreadInfo* GetThreadList();
		struct TTD_Replay_ThreadInfo* GetThreadInfo(unsigned int ThreadId);
		struct TTD_Replay_RegisterContext* GetCrossPlatformContext();
		struct TTD_Replay_RegisterContext* GetCrossPlatformContext(uint32_t threadId);
		struct MemoryBuffer* QueryMemoryBuffer(GuestAddress address, unsigned __int64 size);
		struct TTD_Replay_ICursorView_ReplayResult* ReplayForward(struct TTD_Replay_ICursorView_ReplayResult* replay_result_out, struct Position* posMax, unsigned __int64 stepCount);
		struct TTD_Replay_ICursorView_ReplayResult* ReplayBackward(struct TTD_Replay_ICursorView_ReplayResult* replay_result_out, struct Position* posMin, unsigned __int64 stepCount);
		void SetCallReturnCallback(PROC_CallCallback callCallback, unsigned __int64 callback_value);
		bool AddMemoryWatchpoint(TTD_Replay_MemoryWatchpointData* data);
		bool RemoveMemoryWatchpoint(TTD_Replay_MemoryWatchpointData* data);
		unsigned __int64 GetModuleCount();
		struct TTD_Replay_ModuleInstance* GetModuleList();
	};

	class ReplayEngine {

	private:
		TTD_Replay_ReplayEngine* engine;

	public:
		ReplayEngine();

		/**** Wrapping around the vftable ****/
		bool Initialize(const wchar_t* trace_filename);
		struct TTD_SystemInfo* GetSystemInfo();
		unsigned __int64 GetThreadCount();
		Position* GetFirstPosition();
		Position* GetLastPosition();
		GuestAddress GetPebAddress();
		Cursor NewCursor();
		unsigned __int64 GetModuleCount();
		struct TTD_Replay_Module* GetModuleList();
	};

}