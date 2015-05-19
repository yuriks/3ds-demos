#pragma once
#include <3ds.h>

void draw_menu(const char* choices[], int num_choices, int selected);
u32 interact_menu(int num_choices, int* selected_ptr);
int do_simple_menu(const char* header, const char* choices[], int num_choices);
