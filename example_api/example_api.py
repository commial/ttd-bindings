import pyTTD

# Create a ReplayEngine
eng = pyTTD.ReplayEngine()
# Open the trace
eng.initialize("D:\\traces\\trace.run")

# Retrieve positions
first = eng.get_first_position()
last = eng.get_last_position()
print(f"Trace from {first} to {last}")

# Positions can be compared
assert all([first < last, last > first, first == eng.get_first_position()])

# Print a few information on the trace
peb = eng.get_peb_address()
print(f"Peb is at {peb:x}")
print(f"Thread count {eng.get_thread_count()}")

# Create a cursor, and set it at the beginning of the trace
cursor = eng.new_cursor()
cursor.set_position(first)

# Print a few information
print(f"PC: {cursor.get_program_counter():x}")
thrdinfo = cursor.get_thread_info()
print(f"Thread ID: {thrdinfo.threadid}")
ctxt = cursor.get_crossplatform_context()
print("RCX: %x" % ctxt.rcx)

# Print loaded modules
print(f"Modules ({eng.get_module_count()}):")
for mod in sorted(eng.get_module_list(), key=lambda x: x.base_addr):
    print(f"\t0x{mod.base_addr:x} - 0x{mod.base_addr + mod.image_size:x}\t{mod.path}")

# Print Thread active zones
print(f"Threads ({cursor.get_thread_count()}):")
## Save the current position
position_save = cursor.get_position()
## Get Threads' starting position
threads = {} # Thread ID => {"begin": position, "end": position}
cursor.set_position(first)
for thread in cursor.get_thread_list():
    threads.setdefault(thread.threadid, {})["begin"] = f"{thread.next_major:x}:{thread.next_minor:x}"
## Get Threads' ending position
cursor.set_position(last)
for thread in cursor.get_thread_list():
    threads.setdefault(thread.threadid, {})["end"] = f"{thread.last_major:x}:{thread.last_minor:x}"
## Restore saved position
cursor.set_position(position_save)
## Print out the resulting ranges
for threadid, info in threads.items():
    print(f"\t[0x{threadid:x}] {info['begin']} -> {info['end']}")

# Read the memory
print("@128[RCX]: %s" % cursor.read_mem(ctxt.rcx, 16))
print("@128[RCX+4]: %s" % cursor.read_mem(ctxt.rcx + 4, 16))

# Step forward and backward
print("Step forward: %d" % cursor.replay_forward(2, last))
print(f"PC: {cursor.get_program_counter():x}")
print("Step backward: %d" % cursor.replay_backward(2, first))
print(f"PC: {cursor.get_program_counter():x}")

# Set breakpoints
addr = 0x1c26ac21e40
print(f"Breakpoint r4 at 0x{addr:x}")
bp = pyTTD.MemoryWatchpointData(addr=addr, size=4, flags=pyTTD.BP_FLAGS.READ)
cursor.add_memory_watchpoint(bp)
cursor.replay_forward(pyTTD.MAX_STEP, last)
cursor.remove_memory_watchpoint(bp)
print(f"PC: {cursor.get_program_counter():x}")

addr = 0x7fffcf045418
print(f"Breakpoint X at 0x{addr:x}")
bp = pyTTD.MemoryWatchpointData(addr=addr, size=1, flags=pyTTD.BP_FLAGS.EXEC)
cursor.add_memory_watchpoint(bp)
cursor.replay_forward(pyTTD.MAX_STEP, last)
cursor.remove_memory_watchpoint(bp)
print(f"PC: {cursor.get_program_counter():x}")
