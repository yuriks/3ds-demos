#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static bool dump_process(u32 target_pid, bool dump_ram) {
	bool ret = false;
	Handle process_hnd = 0;
	Handle debug_hnd = 0;
	FILE* file = nullptr;

	log("Dumping process %lu\n", target_pid);

	Result result;
	result = svcOpenProcess(&process_hnd, target_pid);
	log("[%08lX] svcOpenProcess\n", result);
	check(result);

	result = svcDebugActiveProcess(&debug_hnd, target_pid);
	log("[%08lX] svcDebugActiveProcess\n", result);
	check(result);

	if (dump_ram) {
		char filename_buffer[256];
		snprintf(filename_buffer, sizeof(filename_buffer), "process_%03lu.bin", target_pid);
		file = fopen(filename_buffer, "wb");
		if (file == nullptr) {
			log("Failed to open file %s\n", filename_buffer);
			goto cleanup;
		}
	}

	u32 current_segment_addr;
	current_segment_addr = 0x00100000;

	while (true) {
		MemInfo mem_info = {};
		PageInfo page_info = {};
		result = svcQueryProcessMemory(&mem_info, &page_info, process_hnd, current_segment_addr);
		log("[%08lX] query mem: %08lX:\n", result, current_segment_addr);
		printMemoryInfo(&mem_info);
		if (mem_info.state == 0) {
			// 0 == Unmapped
			break;
		}

		if (dump_ram) {
			u8 buffer[0x1000];
			for (u32 p = 0; p < mem_info.size; p += 0x1000) {
				u32 amount = 0x1000;
				result = svcReadProcessMemory(buffer, debug_hnd, current_segment_addr + p, 0x1000);
				if (result & 0x80000000) {
					log("\n[%08lX] svcReadProcessMemory @ %08lX\n", result, current_segment_addr + p);
				}

				if (fwrite(buffer, amount, 1, file) != 1) {
					log("\nFailed to write to file @ %08lX.\n", current_segment_addr + p);
					goto cleanup;
				}

				if (p % 0x100000 == 0) log(".");
			}
			log("\n");
		}

		current_segment_addr = mem_info.base_addr + mem_info.size;
	}

	ret = true;

cleanup:
	if (file != nullptr) fclose(file);
	if (debug_hnd != 0) svcCloseHandle(debug_hnd);
	if (process_hnd != 0) svcCloseHandle(process_hnd);
	return ret;
}

void process_dumper() {
	u32 target_pid = 0;

	u32 self_pid = 0;
	svcGetProcessId(&self_pid, 0xFFFF8001);
	printf("Current process id: %lu\n", self_pid);

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
		if (kDown & KEY_A) {
			fprintf(stderr, "\n");
			dump_process(target_pid, true);
			log("----- Done -----\n");
		}
		if (kDown & KEY_B) {
			fprintf(stderr, "\n");
			dump_process(target_pid, false);
			log("----- Done -----\n");
		}
		if (kDown & KEY_DOWN)
			target_pid -= 10;
		if (kDown & KEY_UP)
			target_pid += 10;
		if (kDown & KEY_LEFT)
			target_pid -= 1;
		if (kDown & KEY_RIGHT)
			target_pid += 1;

		fprintf(stderr, "\rSelect process: %lu              ", target_pid);

		gspWaitForVBlank();
	}
}
