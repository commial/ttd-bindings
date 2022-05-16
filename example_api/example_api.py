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
ctxt = cursor.get_context_x86_64()
print("RCX: %x" % ctxt.rcx)

# Print loaded modules
print(f"Modules ({eng.get_module_count()}):")
for mod in sorted(eng.get_module_list(), key=lambda x: x.base_addr):
    print(f"\t0x{mod.base_addr:x} - 0x{mod.base_addr + mod.image_size:x}\t{mod.path}")

# Print an event based timeline
events = sorted(
    list((x, "modload") for x in eng.get_module_loaded_event_list())
    + list((x, "modunload") for x in eng.get_module_unloaded_event_list())
    + list((x, "threadcreated") for x in eng.get_thread_created_event_list())
    + list((x, "threadterm") for x in eng.get_thread_terminated_event_list()),
    key=lambda event:event[0].position
)
for event, evtype in events:
    print(f"[{event.position.major:x}:{event.position.minor:x}]", end=" ")
    if evtype == "modload":
        print(f"Module {event.info.path} loaded")
    elif evtype == "modunload":
        print(f"Module {event.info.path} unloaded")
    elif evtype == "threadcreated":
        print(f"Thread {event.info.threadid:x} created")
    elif evtype == "threadterm":
        print(f"Thread {event.info.threadid:x} terminated")

# Print threads' future and past active zone
print(f"Threads ({cursor.get_thread_count()}):")
for thread in cursor.get_thread_list():
    print(f"\t[0x{thread.threadid:x}] last active: {thread.last_major:x}:{thread.last_minor:x}, next: {thread.next_major:x}:{thread.next_minor:x}")

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
