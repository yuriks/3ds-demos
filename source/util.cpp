#include <3ds.h>
#include <array>

#include "util.h"

static const char* const MemoryState_names[] = {
	"FREE", "RESERVED", "IO", "STATIC", "CODE", "PRIVATE", "SHARED",
	"CONTINUOUS", "ALIASED", "ALIAS", "ALIAS CODE", "LOCKED"
};

void printMemoryInfo(const MemInfo* mem_info) {
	char r = mem_info->perm & MEMPERM_READ ? 'R' : '-';
	char w = mem_info->perm & MEMPERM_WRITE ? 'W' : '-';
	char x = mem_info->perm & MEMPERM_EXECUTE ? 'X' : '-';

	const char* state_str = "UNKNOWN";
	if (mem_info->state >= 0 && mem_info->state < ARRAY_LEN(MemoryState_names)) {
		state_str = MemoryState_names[mem_info->state];
	}

	log(
			"    addr: %08lX end: %08lX\n"
			"    perm: %lX(%c%c%c) state: %s(%lX) size: %lX\n",
			mem_info->base_addr, mem_info->base_addr + mem_info->size,
			mem_info->perm, r,w,x, state_str, mem_info->state, mem_info->size);
}
