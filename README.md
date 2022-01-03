# libLog

## About

A simple "do it all" logging library (PRX) designed for use on the PS4 using the [OpenOrbis Toolchain]. It should be easy enough to follow along with for beginners as it doesn't do anything "fancy," most of it is just conditionals/data formatting, and it's less than 1,000 lines.

Features include:

- C99 standard, so it should be compatible with your code
- Automatically formats output to display file and line where the log function was called from (Optional)
- Various log levels
  - Set log level to display on-the-fly
- Colorized output based on log level (Optional/Where available)
- Logs to
  - `Print` (Simple printf)
  - `Kernel` (Kernel log, no hooks required)
  - `Socket` (Sent over UDP, does not wait for response or confirmation so it should not hang, but packets can be lost)
  - `File`
- Include a `Hexdump` for dumping arbitrary memory to a human readable format
- Include a `Bindump` for dumping arbitrary memory (Only via `Socket` and `File`)
  - This uses a separate socket/file setup so you won't pollute your logs with binary data

## Notes

- This can be debugged/tested PC side with the include `Makefile.pc` file. Just run `make -f Makefile.pc` to build with it.

## Road to 1.0.0

- [ ] Change `Makefile.pc` to use similar structure/options/flags to `Makefile` so PC side test is closer to actual output
- [ ] Recreate C++ stream bindings
- [ ] Document everything w/ examples
- [ ] Release

## 1.0.0+ Plans

- [ ] System for callbacks to log multiple predefined logs with one log function call
- [ ] Automatic log rotation for logging to files

[//]: #
  [OpenOrbis Toolchain]: <https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain>
