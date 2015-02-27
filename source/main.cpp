#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#define log(...) fprintf(stderr, __VA_ARGS__)

#define wait() svcSleepThread(2000000000ull)
#define check(x) if ((x) & 0x80000000) goto cleanup;

Result __attribute__((naked))
		svcOpenProcess(Handle* process, u32 processId) {
	asm volatile (
			"push {r0}\n\t"
			"svc 0x33\n\t"
			"pop {r2}\n\t"
			"str r1, [r2]\n\t"
			"bx lr\n\t"
			);
}

Result __attribute__((naked))
		svcDebugActiveProcess(Handle* debug, u32 processId) {
	asm volatile (
			"push {r0}\n\t"
			"svc 0x60\n\t"
			"pop {r2}\n\t"
			"str r1, [r2]\n\t"
			"bx lr\n\t"
			);
}

Result __attribute__((naked))
		svcReadProcessMemory(void* buffer, Handle debug, u32 addr, u32 size) {
	asm volatile (
			"svc 0x6A\n\t"
			"bx lr\n\t"
			);
}

Result __attribute__((naked))
		svcCreatePort(Handle* portServer, Handle* portClient, const char* name, s32 maxSessions) {
	asm volatile (
			"push {r0, r1}\t\n"
			"svc 0x47\t\n"
			"ldr r3, [sp, #0]\t\n"
			"str r1, [r3]\t\n"
			"ldr r3, [sp, #4]\t\n"
			"str r2, [r3]\t\n"
			"add sp, sp, #8\t\n"
			"bx lr\t\n");
}

Result __attribute__((naked))
		svcGetProcessList(s32* processCount, u32* processIds, s32 processIdMaxCount) {
	asm volatile (
			"push {r0, r1}\t\n"
			"svc 0x65\t\n"
			"ldr r3, [sp, #0]\t\n"
			"str r1, [r3]\t\n"
			"ldr r3, [sp, #4]\t\n"
			"str r2, [r3]\t\n"
			"add sp, sp, #8\t\n"
			"bx lr\t\n");
}

Result __attribute__((naked))
		svcMapProcessMemory(Handle process, u32 startAddr, u32 endAddr) {
	asm volatile (
			"svc 0x71\n\t"
			"bx lr\n\t"
			);
}

Result __attribute__((naked))
		svcUnmapProcessMemory(Handle process, u32 startAddr, u32 endAddr) {
	asm volatile (
			"svc 0x72\n\t"
			"bx lr\n\t"
			);
}

Result __attribute__((naked))
		svcQueryProcessMemory(MemInfo* info, PageInfo* out, Handle process, u32 addr) {
	asm volatile (
			"push {r0, r1, r4-r6}\n\t"
			"svc 0x7D\n\t"
			"ldr r6, [sp]\n\t"
			"stm r6, {r1-r4}\n\t"
			"ldr r6, [sp, #4]\n\t"
			"str r5, [r6]\n\t"
			"add sp, sp, #8\n\t"
			"pop {r4-r6}\n\t"
			"bx lr\n\t"
			);
}

MemInfo getMemoryInfo(Handle process, u32 addr) {
	MemInfo mem_info = {};
	PageInfo page_info = {};

	u32 result = svcQueryProcessMemory(&mem_info, &page_info, process, addr);
	char r = mem_info.perm & MEMPERM_READ ? 'R' : '-';
	char w = mem_info.perm & MEMPERM_WRITE ? 'W' : '-';
	char x = mem_info.perm & MEMPERM_EXECUTE ? 'X' : '-';
	log(
			"[%08lX] query mem: %08lX:\n"
			"    addr: %08lX end: %08lX\n"
			"    perm: %lX(%c%c%c) state: %lX size: %lX\n",
			result, addr,
			mem_info.base_addr, mem_info.base_addr + mem_info.size,
			mem_info.perm, r,w,x, mem_info.state, mem_info.size);

	return mem_info;
}

bool dump(u32 target_pid, bool dump_ram) {
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

	MemInfo mem_info;
	u32 current_segment_addr;
	current_segment_addr = 0x00100000;

	while (true) {
		MemInfo mem_info = getMemoryInfo(process_hnd, current_segment_addr);
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

u32 waitKey() {

	return 0;
}

int main(int argc, char** argv)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	u32 target_pid = 0;
	if (argc >= 2) target_pid = strtol(argv[1], nullptr, 10);

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
			dump(target_pid, true);
			log("----- Done -----\n");
		}
		if (kDown & KEY_B) {
			fprintf(stderr, "\n");
			dump(target_pid, false);
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

	gfxExit();
	return 0;
}