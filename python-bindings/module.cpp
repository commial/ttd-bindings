#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include "TTD.hpp"


namespace py = pybind11;

PYBIND11_MODULE(pyTTD, m) {
    m.doc() = "Time Travel Debugging (TTD) wrapping";
    m.attr("MAX_STEP") = 0xFFFFFFFFFFFFFFFE;

    py::enum_<TTD::BP_FLAGS>(m, "BP_FLAGS")
        .value("READ", TTD::BP_FLAGS::READ)
        .value("WRITE", TTD::BP_FLAGS::WRITE)
        .value("EXEC", TTD::BP_FLAGS::EXEC)
        .export_values();

    py::class_<TTD::TTD_Replay_MemoryWatchpointData, std::unique_ptr<TTD::TTD_Replay_MemoryWatchpointData, py::nodelete>>(m, "MemoryWatchpointData")
        .def(py::init<TTD::GuestAddress, unsigned __int64, unsigned __int64>(), py::arg("addr"), py::arg("size"), py::arg("flags"));

    py::class_<TTD::TTD_Replay_Module, std::unique_ptr<TTD::TTD_Replay_Module, py::nodelete>>(m, "Module")
        .def_readonly("base_addr", &TTD::TTD_Replay_Module::base_addr)
        .def_readonly("checksum", &TTD::TTD_Replay_Module::checkSum)
        .def_readonly("image_size", &TTD::TTD_Replay_Module::imageSize)
        .def_readonly("path", &TTD::TTD_Replay_Module::path);

    py::class_<TTD::TTD_Replay_ThreadInfo, std::unique_ptr<TTD::TTD_Replay_ThreadInfo, py::nodelete>>(m, "ThreadInfo")
        .def_readonly("threadid", &TTD::TTD_Replay_ThreadInfo::threadid);

    py::class_<TTD::TTD_Replay_Exception, std::unique_ptr<TTD::TTD_Replay_Exception, py::nodelete>>(m, "ExceptionInfo")
        .def_readonly("exception_code", &TTD::TTD_Replay_Exception::ExceptionCode)
        .def_readonly("exception_flags", &TTD::TTD_Replay_Exception::ExceptionFlags)
        .def_readonly("exception_record", &TTD::TTD_Replay_Exception::ExceptionRecord)
        .def_readonly("exception_address", &TTD::TTD_Replay_Exception::ExceptionAddress);

    py::class_<TTD::TTD_Replay_Event<TTD::TTD_Replay_Module*>, std::unique_ptr<TTD::TTD_Replay_Event<TTD::TTD_Replay_Module*>, py::nodelete>>(m, "ModuleEvent")
        .def_readonly("position", &TTD::TTD_Replay_Event<TTD::TTD_Replay_Module*>::pos)
        .def_readonly("info", &TTD::TTD_Replay_Event<TTD::TTD_Replay_Module*>::info);

    py::class_<TTD::TTD_Replay_Event<TTD::TTD_Replay_ThreadInfo*>, std::unique_ptr<TTD::TTD_Replay_Event<TTD::TTD_Replay_ThreadInfo*>, py::nodelete>>(m, "ThreadEvent")
        .def_readonly("position", &TTD::TTD_Replay_Event<TTD::TTD_Replay_ThreadInfo*>::pos)
        .def_readonly("info", &TTD::TTD_Replay_Event<TTD::TTD_Replay_ThreadInfo*>::info);

    py::class_<TTD::TTD_Replay_Event<TTD::TTD_Replay_Exception>, std::unique_ptr<TTD::TTD_Replay_Event<TTD::TTD_Replay_Exception>, py::nodelete>>(m, "ExceptionEvent")
        .def_readonly("position", &TTD::TTD_Replay_Event<TTD::TTD_Replay_Exception>::pos)
        .def_readonly("info", &TTD::TTD_Replay_Event<TTD::TTD_Replay_Exception>::info);
    
    py::class_<TTD::Position, std::unique_ptr<TTD::Position, py::nodelete>>(m, "Position")
        .def_readwrite("major", &TTD::Position::Major)
        .def_readwrite("minor", &TTD::Position::Minor)
        .def("__repr__",
            [](const TTD::Position* pos) {
                char out[256] = { 0 };
                sprintf_s(out, "<Position %llx:%llx>", pos->Major, pos->Minor);
                return std::string(out);
            }
        )
        .def("__lt__", [](TTD::Position &self, const TTD::Position &b) {
                return self < b;
            }, py::is_operator())
        .def("__gt__", [](TTD::Position& self, const TTD::Position& b) {
                return self > b;
        }, py::is_operator())
        .def("__eq__", [](TTD::Position& self, const TTD::Position& b) {
            return (self.Major == b.Major) && (self.Minor == b.Minor);
        }, py::is_operator());

    py::class_<TTD::TTD_Replay_ActiveThreadInfo, std::unique_ptr<TTD::TTD_Replay_ActiveThreadInfo, py::nodelete>>(m, "ActiveThreadInfo")
        .def_property("threadid", [](TTD::TTD_Replay_ActiveThreadInfo& self) {
            return self.info->threadid;
        }, nullptr)
        .def_property("next_major", [](TTD::TTD_Replay_ActiveThreadInfo& self) {
            return self.next.Major;
        }, nullptr)
        .def_property("next_minor", [](TTD::TTD_Replay_ActiveThreadInfo& self) {
            return self.next.Minor;
        }, nullptr)
        .def_property("last_major", [](TTD::TTD_Replay_ActiveThreadInfo& self) {
            return self.last.Major;
        }, nullptr)
        .def_property("last_minor", [](TTD::TTD_Replay_ActiveThreadInfo& self) {
            return self.last.Minor;
        }, nullptr);

    // x86_64 context
    py::class_<CONTEXT, std::unique_ptr<CONTEXT, py::nodelete>>(m, "Context")
        .def_readonly("seg_cs", &CONTEXT::SegCs)
        .def_readonly("seg_ds", &CONTEXT::SegDs)
        .def_readonly("seg_es", &CONTEXT::SegEs)
        .def_readonly("seg_fs", &CONTEXT::SegFs)
        .def_readonly("seg_gs", &CONTEXT::SegGs)
        .def_readonly("seg_ss", &CONTEXT::SegSs)
        .def_readonly("eflags", &CONTEXT::EFlags)
        .def_readonly("rax", &CONTEXT::Rax)
        .def_readonly("rbx", &CONTEXT::Rbx)
        .def_readonly("rcx", &CONTEXT::Rcx)
        .def_readonly("rdx", &CONTEXT::Rdx)
        .def_readonly("rsi", &CONTEXT::Rsi)
        .def_readonly("rdi", &CONTEXT::Rdi)
        .def_readonly("rsp", &CONTEXT::Rsp)
        .def_readonly("rbp", &CONTEXT::Rbp)
        .def_readonly("r8", &CONTEXT::R8)
        .def_readonly("r9", &CONTEXT::R9)
        .def_readonly("r10", &CONTEXT::R10)
        .def_readonly("r11", &CONTEXT::R11)
        .def_readonly("r12", &CONTEXT::R12)
        .def_readonly("r13", &CONTEXT::R13)
        .def_readonly("r14", &CONTEXT::R14)
        .def_readonly("r15", &CONTEXT::R15)
        .def_readonly("rip", &CONTEXT::Rip)
        .def_readonly("fltsave", &CONTEXT::FltSave);

    // x86 context
    py::class_<WOW64_CONTEXT, std::unique_ptr<WOW64_CONTEXT, py::nodelete>>(m, "WOW64_Context")
        .def_readonly("seg_cs", &WOW64_CONTEXT::SegCs)
        .def_readonly("seg_ds", &WOW64_CONTEXT::SegDs)
        .def_readonly("seg_es", &WOW64_CONTEXT::SegEs)
        .def_readonly("seg_fs", &WOW64_CONTEXT::SegFs)
        .def_readonly("seg_gs", &WOW64_CONTEXT::SegGs)
        .def_readonly("seg_ss", &WOW64_CONTEXT::SegSs)
        .def_readonly("eflags", &WOW64_CONTEXT::EFlags)
        .def_readonly("eax", &WOW64_CONTEXT::Eax)
        .def_readonly("ebx", &WOW64_CONTEXT::Ebx)
        .def_readonly("ecx", &WOW64_CONTEXT::Ecx)
        .def_readonly("edx", &WOW64_CONTEXT::Edx)
        .def_readonly("esi", &WOW64_CONTEXT::Esi)
        .def_readonly("edi", &WOW64_CONTEXT::Edi)
        .def_readonly("esp", &WOW64_CONTEXT::Esp)
        .def_readonly("ebp", &WOW64_CONTEXT::Ebp)
        .def_readonly("eip", &WOW64_CONTEXT::Eip);


    py::class_<TTD::Cursor>(m, "Cursor")
        .def("set_position", py::overload_cast<TTD::Position*>(&TTD::Cursor::SetPosition), py::arg("position"))
        .def("set_position", py::overload_cast<unsigned __int64, unsigned __int64>(&TTD::Cursor::SetPosition), py::arg("Major"), py::arg("Minor"))
        .def("get_position", py::overload_cast<>(&TTD::Cursor::GetPosition))
        .def("get_position", py::overload_cast<uint32_t>(&TTD::Cursor::GetPosition), py::arg("threadid"))
        .def("get_thread_count", &TTD::Cursor::GetThreadCount)
        .def("get_program_counter", py::overload_cast<>(&TTD::Cursor::GetProgramCounter))
        .def("get_thread_info", py::overload_cast<>(&TTD::Cursor::GetThreadInfo))
        .def("get_crossplatform_context", py::overload_cast<>(&TTD::Cursor::GetCrossPlatformContext))
        .def("get_crossplatform_context", py::overload_cast<uint32_t>(&TTD::Cursor::GetCrossPlatformContext), py::arg("threadid"))
        .def("get_context_x86_64", py::overload_cast<>(&TTD::Cursor::GetContextx86_64))
        .def("get_context_x86_64", py::overload_cast<uint32_t>(&TTD::Cursor::GetContextx86_64), py::arg("threadid"))
        .def("get_context_x86", py::overload_cast<>(&TTD::Cursor::GetContextx86))
        .def("get_context_x86", py::overload_cast<uint32_t>(&TTD::Cursor::GetContextx86), py::arg("threadid"))
        .def("read_mem", [](TTD::Cursor &self, TTD::GuestAddress addr, unsigned __int64 size) {
            TTD::MemoryBuffer *membuf = self.QueryMemoryBuffer(addr, size);
            std::string x((char*) membuf->data, membuf->size);
            free(membuf->data);
            free(membuf);
            return py::bytes(x);
            })
        .def("replay_forward", [](TTD::Cursor& self, unsigned __int64 stepCount, TTD::Position* last) {
            TTD::TTD_Replay_ICursorView_ReplayResult replayrez;
            self.ReplayForward(&replayrez, last, stepCount);
            return replayrez.stepCount;
        })
        .def("replay_backward", [](TTD::Cursor& self, unsigned __int64 stepCount, TTD::Position* first) {
            TTD::TTD_Replay_ICursorView_ReplayResult replayrez;
            self.ReplayBackward(&replayrez, first, stepCount);
            return replayrez.stepCount;
        })
        .def("add_memory_watchpoint", &TTD::Cursor::AddMemoryWatchpoint)
        .def("remove_memory_watchpoint", &TTD::Cursor::RemoveMemoryWatchpoint)
        .def("get_module_count", &TTD::Cursor::GetModuleCount)
        .def("get_module_list", [](TTD::Cursor& self) {
            std::vector<TTD::TTD_Replay_Module> mods;
            TTD::TTD_Replay_ModuleInstance* mod_list = self.GetModuleList();
            for (int i = 0; i < self.GetModuleCount(); i++) {
                mods.push_back(*mod_list[i].module);
            }
            return mods;
        })
        .def("get_thread_list", [](TTD::Cursor& self) {
            std::vector<TTD::TTD_Replay_ActiveThreadInfo> threads;
            TTD::TTD_Replay_ActiveThreadInfo* thread_list = self.GetThreadList();
            for (int i = 0; i < self.GetThreadCount(); i++) {
                threads.push_back(thread_list[i]);
            }
            return threads;
        });
        
    py::class_<TTD::ReplayEngine>(m, "ReplayEngine")
        .def(py::init<>())
        .def("initialize", &TTD::ReplayEngine::Initialize, py::arg("trace_filename"))
        .def("get_thread_count", &TTD::ReplayEngine::GetThreadCount)
        .def("get_first_position", &TTD::ReplayEngine::GetFirstPosition)
        .def("get_last_position", &TTD::ReplayEngine::GetLastPosition)
        .def("get_peb_address", &TTD::ReplayEngine::GetPebAddress)
        .def("new_cursor", &TTD::ReplayEngine::NewCursor)
        .def("get_module_loaded_event_count", &TTD::ReplayEngine::GetModuleLoadedEventCount)
        .def("get_module_loaded_event_list", [](TTD::ReplayEngine& self) {
            std::vector<TTD::TTD_Replay_ModuleLoadedEvent> mods;
            const TTD::TTD_Replay_ModuleLoadedEvent* mod_list = self.GetModuleLoadedEventList();
            for (int i = 0; i < self.GetModuleLoadedEventCount(); i++) {
                mods.push_back(mod_list[i]);
            }
            return mods;
        })
        .def("get_module_unloaded_event_count", &TTD::ReplayEngine::GetModuleUnloadedEventCount)
        .def("get_module_unloaded_event_list", [](TTD::ReplayEngine& self) {
            std::vector<TTD::TTD_Replay_ModuleUnloadedEvent> mods;
            const TTD::TTD_Replay_ModuleUnloadedEvent* mod_list = self.GetModuleUnloadedEventList();
            for (int i = 0; i < self.GetModuleUnloadedEventCount(); i++) {
                mods.push_back(mod_list[i]);
            }
            return mods;
        })
        .def("get_thread_created_event_count", &TTD::ReplayEngine::GetThreadCreatedEventCount)
        .def("get_thread_created_event_list", [](TTD::ReplayEngine& self) {
            std::vector<TTD::TTD_Replay_ThreadCreatedEvent> thrds;
            const TTD::TTD_Replay_ThreadCreatedEvent* thrd_list = self.GetThreadCreatedEventList();
            for (int i = 0; i < self.GetThreadCreatedEventCount(); i++) {
                thrds.push_back(thrd_list[i]);
            }
            return thrds;
        })
        .def("get_thread_terminated_event_count", &TTD::ReplayEngine::GetThreadTerminatedEventCount)
        .def("get_thread_terminated_event_list", [](TTD::ReplayEngine& self) {
            std::vector<TTD::TTD_Replay_ThreadTerminatedEvent> thrds;
            const TTD::TTD_Replay_ThreadTerminatedEvent* thrd_list = self.GetThreadTerminatedEventList();
            for (int i = 0; i < self.GetThreadTerminatedEventCount(); i++) {
                thrds.push_back(thrd_list[i]);
            }
            return thrds;
        })
        .def("get_exception_event_count", &TTD::ReplayEngine::GetExceptionEventCount)
        .def("get_exception_event_list", [](TTD::ReplayEngine& self) {
            std::vector<TTD::TTD_Replay_ExceptionEvent> excps;
            const TTD::TTD_Replay_ExceptionEvent* excp_list = self.GetExceptionEventList();
            for (int i = 0; i < self.GetExceptionEventCount(); i++) {
                excps.push_back(excp_list[i]);
            }
            return excps;
        })
        .def("get_module_count", &TTD::ReplayEngine::GetModuleCount)
        .def("get_module_list", [](TTD::ReplayEngine& self) {
            std::vector<TTD::TTD_Replay_Module> mods;
            const TTD::TTD_Replay_Module* mod_list = self.GetModuleList();
            for (int i = 0; i < self.GetModuleCount(); i++) {
                mods.push_back(mod_list[i]);
            }
            return mods;
        });


#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
