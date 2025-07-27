# Fakegrind

An attempt to recreate **Valgrind** in Windows, taking inspiration from the (now unmaintained) **WinValgrind**.

### How it works?

Fakegrind injects onto the target process (which can be started by Fakegrind, or a PID can be passed onto it).

Once it is injected, it tracks API calls such as `OpenProcess`, `malloc`, ..., which allocate resources, keeping track of whether they're being cleaned up, reporting it to the user/ developer so they can take a peek into it.

