# 3ds-process-dumper
Allows dumping of memory from other processes in a 3DS. Use the `debug-flag` branch of https://github.com/yuriks/bootstrap/tree/debug-flag first to unlock SVC calls and debugging privileges.

Use d-pad to select process id. B to display only the memory map, A to dump the process image.
The process text, data and bss will be written out to a file `process_XXX.bin` on the SD. Currently the stack and heap are not dumped. It is also not currently possible to dump processes using the special memory layout.

**NOTE:** Trying to deadlock some processes (including yourself, the invalid process 0, and a few others) will deadlock the system.
