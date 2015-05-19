#pragma once
#include <3ds.h>
#include <stdio.h>

#define log(...) fprintf(stderr, __VA_ARGS__)

#define wait() svcSleepThread(2000000000ull)
#define check(x) if ((x) & 0x80000000) goto cleanup;

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(*(arr)))

void printMemoryInfo(const MemInfo* mem_info);
u32 wait_input();
