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


    py::class_<TTD::Position, std::unique_ptr<TTD::Position, py::nodelete>>(m, "Position")
        .def_readwrite("major", &TTD::Position::Major)
        .def_readwrite("minor", &TTD::Position::Minor)
        .def("__repr__",
            [](const TTD::Position* pos) {
                char out[256] = { 0 };
                sprintf_s(out, "<Position %llx:%llx>", pos->Major, pos->Minor);
                return std::string(out);
            }
    );

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

    py::class_<CONTEXT, std::unique_ptr<CONTEXT, py::nodelete>>(m, "Context")
        .def_readonly("SegCs", &CONTEXT::SegCs)
        .def_readonly("SegDs", &CONTEXT::SegDs)
        .def_readonly("SegEs", &CONTEXT::SegEs)
        .def_readonly("SegFs", &CONTEXT::SegFs)
        .def_readonly("SegGs", &CONTEXT::SegGs)
        .def_readonly("SegSs", &CONTEXT::SegSs)
        .def_readonly("EFlags", &CONTEXT::EFlags)
        .def_readonly("Rax", &CONTEXT::Rax)
        .def_readonly("Rbx", &CONTEXT::Rbx)
        .def_readonly("Rcx", &CONTEXT::Rcx)
        .def_readonly("Rdx", &CONTEXT::Rdx)
        .def_readonly("Rsi", &CONTEXT::Rsi)
        .def_readonly("Rdi", &CONTEXT::Rdi)
        .def_readonly("Rsp", &CONTEXT::Rsp)
        .def_readonly("Rbp", &CONTEXT::Rbp)
        .def_readonly("R8", &CONTEXT::R8)
        .def_readonly("R9", &CONTEXT::R9)
        .def_readonly("R10", &CONTEXT::R10)
        .def_readonly("R11", &CONTEXT::R11)
        .def_readonly("R12", &CONTEXT::R12)
        .def_readonly("R13", &CONTEXT::R13)
        .def_readonly("R14", &CONTEXT::R14)
        .def_readonly("R15", &CONTEXT::R15)
        .def_readonly("Rip", &CONTEXT::Rip)
        .def_readonly("FltSave", &CONTEXT::FltSave);


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
