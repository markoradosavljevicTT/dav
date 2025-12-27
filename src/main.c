#include "ui.h"
#include "visualizer.h"
#include <gio/gio.h>

int main(int argc, char *argv[]) {
	AppData *data = calloc(1, sizeof(AppData));
	data->stream = calloc(1, sizeof(StreamData));
	data->settings = read_config();
	data->visualizer = false;
	data->gifs = NULL;

	// Set GSettings schema directory for macOS
	g_setenv("GSETTINGS_SCHEMA_DIR", "/opt/homebrew/share/glib-2.0/schemas", TRUE);
	gtk_init(&argc, &argv);
	create_window(data);
	open_overlay(data);
	gtk_main();

	return 0;
}

