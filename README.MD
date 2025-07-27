# Fakegrind

An attempt to recreate **Valgrind** in Windows, taking inspiration from the (now unmaintained) **WinValgrind**.

### How it works?

Fakegrind injects onto the target process (which can be started by Fakegrind, or a PID can be passed onto it).

Once it is injected, it tracks calls to memory allocators `malloc`, `free`, `realloc`, `calloc` (and more when their
services' are implemented)

After it finds bad calls, it reports them.

By default, Fakegrind does not generate stack traces, as it makes the program run a lot slower due to walking the stack.

### Compiling

This project can be compiled using `clang`, `clang-cl` or `cl`/`msvc`. It should work out of the box just doing cmake
generation with one of the presets should do!

### Remarks

This project is ONLY meant for compilation on X86_64, we do NOT support 32 bit applications, as they're quickly becoming
less and less used, and supporting them is an unnecessary endeavour.