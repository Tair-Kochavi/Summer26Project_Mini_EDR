# Mini-EDR  

## Project Overview
`Mini-EDR` is a diagnostic and active runtime monitor for Linux (x86_64) that observes
and tampers with interactions between processes and the Linux kernel. It traces system calls, process state changes,
and memory allocation in real-time using the kernel's `ptrace` utility.

Unlike passive tracers, `Mini-EDR` functions as an active enforcement sandbox.
It extracts and parses virtual memory across process boundaries, evaluates parameters
against security policies, and utilizes two-stage register manipulation (`PTRACE_SETREGS`) to actively sabotage
malicious kernel requests and simulate native OS error codes (`-EACCES`, `-EPERM`).

### Core Subsystems
* **Multi-Process & Thread tracing:** Tracks dynamic execution trees concurrently. Configured with `PTRACE_O_TRACEFORK`,
`PTRACE_O_TRACEVFORK`, `PTRACE_O_TRACECLONE` and `PTRACE_O_TRACEEXEC` to maintain independent state machines across nested
child processes and thread interleaving without state collisions.
* **W^X Shellcode Prevention:** Intercepts `mmap` and `mprotect` to prevent memory-based code injection by blocking 
concurrent `PROT_WRITE` and `PROT_EXEC` page allocation.
*  **Deep Memory Inspector:** Safely extracts null-terminated strings (`PATH_MAX` guarded) and raw socket structures
   (`AF_INET`, `AF_INET6`, `AF_UNIX`) directly from tracee memory (`PTRACE_PEEKDATA`).



## Build System
The project uses CMake to handle compilation and linking. It requires a Linux environment with a C++17 compatible compiler
(GCC or Clang).
If you are not using an IDE like CLion (which builds automatically), you can compile the sandbox via terminal:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Usage
Pass the target executable and its arguments directly to the tracer:
```bash
./mini_edr <target_executable> [args...]
```

## Testing Architecture
The repository includes a comprehensive, multi-phase testing directory designed to validate kernel edge cases, memory safety,
and signal handling. This directory includes:

1. Lifecycle & Signal Handling Test (`test_process_lifecycle.cpp`): Validates orphaned child handling, `vfork` tracking, and
`WIFSIGNALED` trap handling for unexpected `SIGSEGV` and `SIGKILL` process terminations.
2. Thread Interleaving (`test_threads_interleave.cpp`): Validates state machine isolation during simultaneous `pthread` system 
call traps.
3. File & Memory Edge Cases (`test_file_edge_cases.cpp`): Verifies relative-path mitigations, trailing-slash bypass prevention,
unmapped syscall filtering, and `NULL` pointer/buffer overflow safety nets.
4. Socket & Connection Inspector Test (`test_socket_edge_cases.cpp`): Tests Endianness translation and blocks across IPv4, IPv6 
and local `AF_UNIX` IPC sockets.
5. Shellcode preventer (`test_shellcode_wx.cpp`): Validates Write-XOR-Execute (W^X) enforcement and verifies clean `execve` 
memory-wipe (`PTRACE_EVENT_EXEC`).

To run a test, pass the compiled test binary to the EDR: 
```bash
./mini_edr ./test_file_edge_cases
```

## Directory structure
* **`main.cpp`** - Core ptrace event loop, signal handling, register sabotage/spoofing, and virtual memory extraction.
* **`syscall_format.cpp / .h`** - Syscall map, dynamic linker noise filtering, and structured standard output formatting.
*  **`blacklist.cpp / .h`** - Filesystem security policies and path-filtering logic.
*  **`nw_blacklist.cpp / .h`** - Network security policies, socket struct parsing, and IP translation.
*  **`tests/`** - Directory containing the standalone C++ tests.
