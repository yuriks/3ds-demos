#include <3ds.h>
#include <stdio.h>

#include "menu.h"
#include "util.h"

void draw_menu(const char* choices[], int num_choices, int selected) {
	for (int i = 0; i < num_choices; ++i) {
		char cursor = selected == i ? '>' : ' ';
		fprintf(stdout, "%c %s\n", cursor, choices[i]);
	}
}

void interact_menu(u32 keys, int num_choices, int* selected_ptr) {
	int selected = *selected_ptr;

	if (keys & KEY_UP)
		selected -= 1;
	if (keys & KEY_DOWN)
		selected += 1;

	if (selected < 0)
		selected = num_choices - 1;
	if (selected >= num_choices)
		selected = 0;

	*selected_ptr = selected;
}

int do_simple_menu(const char* header, const char* choices[], int num_choices) {
	int selected = 0;

	while (aptMainLoop()) {
		gspWaitForVBlank();
		consoleClear();
		fprintf(stdout, "%s", header);
		draw_menu(choices, num_choices, selected);

		u32 keys = wait_input();
		interact_menu(keys, num_choices, &selected);
		if (keys & KEY_A)
			return selected;
		if (keys & KEY_B)
			break;
	}

	return -1;
}
