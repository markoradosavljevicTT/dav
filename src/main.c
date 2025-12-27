#include "data.h"
#include "ui.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
	AppData *data = calloc(1, sizeof(AppData));
	data->gifs = NULL;

	data->native_window = ui_create_settings_window(data);
	ui_show_settings_window(data->native_window);

	data->overlay = ui_create_overlay(data);

	ui_start_app_loop();

	return 0;
}
