#include <3ds.h>

#include "util.h"

void address_space_info() {
	int wait_counter = 0;
	u32 addr = 0;
	while (true) {
		Result result;
		MemInfo mem_info = {};
		PageInfo page_info = {};

		result = svcQueryMemory(&mem_info, &page_info, addr);
		log(C_GREEN "[%08lX] svcQueryMemory(%08lX)\n" C_RESET, result, addr);
		if (result & 0x80000000)
			break;

		printMemoryInfo(&mem_info);

		if (++wait_counter == 9) {
			wait_counter = 0;
			fprintf(stdout, "--- MORE ---\n");
			wait_input();
		}

		if (addr + mem_info.size < addr)
			break;
		addr += mem_info.size;
	}
}
