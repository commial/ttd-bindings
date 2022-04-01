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
    py::class_<TTD::TTD_Replay_ActiveThreadInfo, std::unique_ptr<TTD::TTD_Replay_ActiveThreadInfo, py::nodelete>>(m, "ActiveThreadInfo")
        .def_property("threadid", [](TTD::TTD_Replay_ActiveThreadInfo& self) {
            return self.info->threadid;
        }, nullptr)

    py::class_<TTD::TTD_Replay_RegisterContext, std::unique_ptr<TTD::TTD_Replay_RegisterContext, py::nodelete>>(m, "RegisterContext")
        .def_readonly("cs", &TTD::TTD_Replay_RegisterContext::cs)
        .def_readonly("ds", &TTD::TTD_Replay_RegisterContext::ds)
        .def_readonly("es", &TTD::TTD_Replay_RegisterContext::es)
        .def_readonly("fs", &TTD::TTD_Replay_RegisterContext::fs)
        .def_readonly("gs", &TTD::TTD_Replay_RegisterContext::gs)
        .def_readonly("ss", &TTD::TTD_Replay_RegisterContext::ss)
        .def_readonly("efl", &TTD::TTD_Replay_RegisterContext::efl)
        .def_readonly("rax", &TTD::TTD_Replay_RegisterContext::rax)
        .def_readonly("rbx", &TTD::TTD_Replay_RegisterContext::rbx)
        .def_readonly("rcx", &TTD::TTD_Replay_RegisterContext::rcx)
        .def_readonly("rdx", &TTD::TTD_Replay_RegisterContext::rdx)
        .def_readonly("rsi", &TTD::TTD_Replay_RegisterContext::rsi)
        .def_readonly("rdi", &TTD::TTD_Replay_RegisterContext::rdi)
        .def_readonly("rsp", &TTD::TTD_Replay_RegisterContext::rsp)
        .def_readonly("rbp", &TTD::TTD_Replay_RegisterContext::rbp)
        .def_readonly("r8", &TTD::TTD_Replay_RegisterContext::r8)
        .def_readonly("r9", &TTD::TTD_Replay_RegisterContext::r9)
        .def_readonly("r10", &TTD::TTD_Replay_RegisterContext::r10)
        .def_readonly("r11", &TTD::TTD_Replay_RegisterContext::r11)
        .def_readonly("r12", &TTD::TTD_Replay_RegisterContext::r12)
        .def_readonly("r13", &TTD::TTD_Replay_RegisterContext::r13)
        .def_readonly("r14", &TTD::TTD_Replay_RegisterContext::r14)
        .def_readonly("r15", &TTD::TTD_Replay_RegisterContext::r15)
        .def_readonly("rip", &TTD::TTD_Replay_RegisterContext::rip)
        .def_readonly("fpcw", &TTD::TTD_Replay_RegisterContext::fpcw);


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
            TTD::TTD_Replay_Module* mod_list = self.GetModuleList();
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
