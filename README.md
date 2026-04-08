# libLog

## Overview

`libLog` is a lightweight, PS4-focused logging library built for the [OpenOrbis Toolchain]. It provides a flexible logging interface for static `.a` libraries or PRX modules, and is intentionally simple so it is easy to read, maintain, and adapt.

The library is implemented in plain C99 and keeps the codebase around 1,000 lines while still offering multiple backends, log levels, formatted output, and memory dump helpers.

## Key Features

- C99-compatible API for easy integration with C/C++ projects
- Supports multiple output backends:
  - `Print` for standard printf-style output
  - `Kernel` for PS4 kernel logs
  - `Socket` for UDP-based remote logging
  - `File` for persistent log files
- Optional file/line formatting in log messages
- Optional colorized output where terminal support is available
- Log levels for controlling verbosity at runtime
- `Hexdump` support for human-readable memory dumps
- `Bindump` support for raw binary dumps via socket or file
- PC-side debugging support through `Makefile.pc`

## Why Use `libLog`?

`libLog` is built for developers who want:

- a simple, dependency-free logging layer for PS4 homebrew or debugging
- easy switching between console, kernel, network, and file logging
- a small codebase that is easy to inspect and extend
- support for both static linking and PRX packaging

## Build Instructions

### PS4 / OpenOrbis

Set `OO_PS4_TOOLCHAIN` to your OpenOrbis toolchain path and run:

```sh
make
```

This builds:

- `libLog.prx`
- `libLog.a`
- `libLog_stub.so`

### PC Debug/Test Build

For host-side testing, use the provided PC makefile:

```sh
make -f Makefile.pc
```

This builds a local `libLog` executable that can be used to verify behavior without PS4 deployment.

## Example Usage

A simple example is available in `src/pc-example.c`. The library exposes functions such as:

- `logPrint(...)`
- `logKernel(...)`
- `logSocket(...)`
- `logFile(...)`
- `logPrintHexdump(...)`
- `logSocketBindump(...)`
- `logFileBindump(...)`

A typical workflow is:

```c
logPrint(LL_Info, "Hello from libLog: %s", "startup complete");
logSocketOpen("192.0.2.2", 9023);
logSocket(LL_Error, "Remote log test");
logSocketClose();
```

## Testing & Development

- Use `make -f Makefile.pc` to build the PC test binary
- Use `make clean` to remove build artifacts
- `Makefile` supports `install` for copying the static library into the OpenOrbis toolchain lib folder

## Roadmap

- [ ] Explore PS5 support
- [ ] Improve test coverage for all backends
- [ ] Add more usage examples and documentation
- [ ] Add automatic file rotation for file logging
- [ ] Add callback-based batch logging helpers

## Notes

- `Makefile.pc` is intended for local debugging and development on Linux/macOS
- Socket logging is fire-and-forget; packets may be dropped if the network is unreliable
- File and bindump output are separated so raw binary data does not pollute text logs

## License

Released under the terms of the project [license](LICENSE).

[OpenOrbis Toolchain]: <https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain>
