#include "ui.h"
#include <stdbool.h>
#include <stdio.h>
#include "gif.h"

#if defined(__APPLE__)
#include "macos_helper.h"
#endif

void show_notification(AppData* data, const char* message) {
	GtkWidget* dialog = gtk_dialog_new_with_buttons("Custom Popup", GTK_WINDOW(data->window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", GTK_RESPONSE_OK, NULL);
	GtkWidget* label = gtk_label_new(message);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label, TRUE, TRUE, 0);
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void on_gif_tab_destroyed(GtkWidget* tab_content, gpointer user_data) {
	AppData* data = (AppData*)user_data;
	Gif* gif = g_object_get_data(G_OBJECT(tab_content), "gif_ptr");
	if (gif) {
		data->gifs = g_list_remove(data->gifs, gif);
		gif_free(gif);
	}
}

void set_gif_x(GtkSpinButton* spin, gpointer user_data) {
	Gif* gif = (Gif*)user_data;
	gif->gif_x = gtk_spin_button_get_value_as_int(spin);
}

void set_gif_y(GtkSpinButton* spin, gpointer user_data) {
	Gif* gif = (Gif*)user_data;
	gif->gif_y = gtk_spin_button_get_value_as_int(spin);
}

void set_gif_width(GtkSpinButton* spin, gpointer user_data) {
	Gif* gif = (Gif*)user_data;
	gif->gif_width = gtk_spin_button_get_value_as_int(spin);
}

void set_gif_height(GtkSpinButton* spin, gpointer user_data) {
	Gif* gif = (Gif*)user_data;
	gif->gif_height = gtk_spin_button_get_value_as_int(spin);
}

static void close_tab(GtkButton* button, gpointer user_data) {
	GtkNotebook* notebook = GTK_NOTEBOOK(user_data);
	AppData* data = g_object_get_data(G_OBJECT(notebook), "app_data");

	int num_pages = gtk_notebook_get_n_pages(notebook);
	for (int i = 0; i < num_pages; i++) {
		GtkWidget* page = gtk_notebook_get_nth_page(notebook, i);
		GtkWidget* tab_label = gtk_notebook_get_tab_label(notebook, page);

		if (tab_label && gtk_widget_is_ancestor(GTK_WIDGET(button), tab_label)) {
			gulong handler_id = g_signal_handler_find(button, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, close_tab, NULL);
			if (handler_id > 0) {
				g_signal_handler_disconnect(button, handler_id);
			}

			Gif* gif = g_object_get_data(G_OBJECT(page), "gif_ptr");
			if (gif && data) {
				data->gifs = g_list_remove(data->gifs, gif);
				gif_free(gif);
			}

			gtk_notebook_remove_page(notebook, i);
			break;
		}
	}
}

void open_gif_tab(AppData* data, void* g) {
	Gif* gif = (Gif*)g;

	GtkWidget* tab_content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_container_set_border_width(GTK_CONTAINER(tab_content), 10);

	GtkWidget* controls_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_box_pack_start(GTK_BOX(tab_content), controls_container, FALSE, FALSE, 0);

	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
	gtk_box_pack_start(GTK_BOX(controls_container), grid, FALSE, FALSE, 0);

	int row = 0;

	GtkWidget* spin_x = gtk_spin_button_new_with_range(-5000, 5000, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_x), gif->gif_x);
	g_signal_connect(spin_x, "value-changed", G_CALLBACK(set_gif_x), gif);
	gtk_grid_attach(GTK_GRID(grid), gtk_label_new("X Position"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), spin_x, 1, row++, 1, 1);

	GtkWidget* spin_y = gtk_spin_button_new_with_range(-5000, 5000, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_y), gif->gif_y);
	g_signal_connect(spin_y, "value-changed", G_CALLBACK(set_gif_y), gif);
	gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Y Position"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), spin_y, 1, row++, 1, 1);

	GtkWidget* spin_w = gtk_spin_button_new_with_range(1, 5000, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_w), gif->gif_width);
	g_signal_connect(spin_w, "value-changed", G_CALLBACK(set_gif_width), gif);
	gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Width"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), spin_w, 1, row++, 1, 1);

	GtkWidget* spin_h = gtk_spin_button_new_with_range(1, 5000, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_h), gif->gif_height);
	g_signal_connect(spin_h, "value-changed", G_CALLBACK(set_gif_height), gif);
	gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Height"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), spin_h, 1, row++, 1, 1);

	GtkWidget* preview_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(tab_content), preview_container, TRUE, TRUE, 0);

	GtkWidget* event_box = gtk_event_box_new();
	gtk_widget_set_size_request(event_box, 400, 300);
	gtk_widget_set_halign(event_box, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(event_box, GTK_ALIGN_CENTER);
	gtk_box_pack_start(GTK_BOX(preview_container), event_box, TRUE, TRUE, 0);

	GtkWidget* image = gtk_image_new_from_animation(gif->gif_anim);
	gtk_container_add(GTK_CONTAINER(event_box), image);

	GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	GtkWidget* label = gtk_label_new(g_path_get_basename(gif->path));
	GtkWidget* close_btn = gtk_button_new_with_label("×");
	gtk_button_set_relief(GTK_BUTTON(close_btn), GTK_RELIEF_NONE);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), close_btn, FALSE, FALSE, 0);
	gtk_widget_show_all(hbox);

	gtk_widget_show_all(tab_content);
	int page = gtk_notebook_append_page(GTK_NOTEBOOK(data->notebook), tab_content, hbox);
	gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(data->notebook), tab_content, TRUE);

	g_signal_connect(close_btn, "clicked", G_CALLBACK(close_tab), data->notebook);
	g_object_set_data(G_OBJECT(tab_content), "gif_ptr", gif);
	g_signal_connect(tab_content, "destroy", G_CALLBACK(on_gif_tab_destroyed), data);

	gtk_widget_show_all(data->notebook);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->notebook), page);
}

void add_gif_tab(AppData* data, const char* path) {
	if (!data->notebook)
		return;

	Gif* gif = gif_new(path);

	if (gif->gif_width > 512 || gif->gif_height > 512) {
		char error_message[256];
		snprintf(error_message, sizeof(error_message), 
				 "GIF je prevelik!\nDimenzije: %dx%d\nMaksimalne dozvoljene dimenzije: %dx%d", 
				 gif->gif_width, gif->gif_height, 512, 512);

		show_notification(data, error_message);
		g_free(gif);
		return;
	}

	data->gifs = g_list_append(data->gifs, gif);
	open_gif_tab(data, gif);
}

void on_add_gif_clicked(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	GtkWidget* dialog = gtk_file_chooser_dialog_new("Select GIF File",
													GTK_WINDOW(data->window),
													GTK_FILE_CHOOSER_ACTION_OPEN,
													"_Cancel", GTK_RESPONSE_CANCEL,
													"_Open", GTK_RESPONSE_ACCEPT,
													NULL);

	if (!dialog) return;

	GtkFileFilter* filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.gif");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (filename) {
			add_gif_tab(data, filename);
			g_free(filename);
		}
	}
	gtk_widget_destroy(dialog);
}

void select_device(GtkWidget* widget, gpointer d) {
	if (GTK_IS_COMBO_BOX(widget)) {
		const char* id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget));
		if (id) {
			device = atoi(id);
			AppData* data = (AppData*)d;
			const PaDeviceInfo* di = Pa_GetDeviceInfo(device);
			if (di) {
				gtk_label_set_text(GTK_LABEL(data->device_name), di->name);
			}
		}
		return;
	}

	int data_int = GPOINTER_TO_INT(d);
	device = data_int;
}

void start_stop(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	if (data->visualizer == false) {
		if(device < 0 || device > Pa_GetDeviceCount()) {
			show_notification(data, "\tSelect valid source\t");
			return;
		}
		data->visualizer = true;
		data->stream->device = device;
		start_stream(data->stream);
		gtk_label_set_text(GTK_LABEL(data->device_name), Pa_GetDeviceInfo(device)->name);
		gtk_button_set_label(GTK_BUTTON(data->start_stop), "Stop");
	}
	else {
		gtk_button_set_label(GTK_BUTTON(data->start_stop), "Start");
		close_stream(data->stream);
		data->visualizer = false;
	}
}

void refresh(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	if(data->visualizer) {
		start_stop(NULL, data);
	}

	refresh_stream(data->stream);

	if (GTK_IS_COMBO_BOX_TEXT(data->devices)) {
		gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(data->devices));
		int cnt = Pa_GetDeviceCount();
		const PaDeviceInfo* device_info;
		for (int dev = 0; dev < cnt; dev++) {
			device_info = Pa_GetDeviceInfo(dev);
			if (device_info->maxInputChannels > 0 && device_info->maxInputChannels < 8) {
				char idbuf[32];
				snprintf(idbuf, sizeof(idbuf), "%d", dev);
				gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(data->devices), idbuf, device_info->name);
			}
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->devices), -1);
		device = -1;
		return;
	}

	GList* children = gtk_container_get_children(GTK_CONTAINER(data->devices));
	for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
		GtkWidget* menuItem = GTK_WIDGET(iter->data);
		gtk_widget_destroy(menuItem);
	}
	g_list_free(children);

	int cnt = Pa_GetDeviceCount();
	const PaDeviceInfo* device_info;
	for (int device = 0; device < cnt; device++) {
		device_info = Pa_GetDeviceInfo(device);
		if(device_info->maxInputChannels > 0 && device_info->maxInputChannels < 8) {
			GtkWidget* menuItem = gtk_menu_item_new_with_label(device_info->name);
			g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(select_device), GINT_TO_POINTER(device));
			gtk_menu_shell_append(GTK_MENU_SHELL(data->devices), menuItem);
			gtk_widget_show(menuItem);
		}
	}
	device = -1;
}

void set_radius(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	data->settings->radius = gtk_range_get_value(GTK_RANGE(widget));
	write_config(data->settings);
}

void set_space(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	data->settings->space = gtk_range_get_value(GTK_RANGE(widget));
	write_config(data->settings);
}

void set_speed(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	data->stream->speed = gtk_range_get_value(GTK_RANGE(widget));
	data->settings->speed = gtk_range_get_value(GTK_RANGE(widget));
	write_config(data->settings);
}

void set_red(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	data->settings->red = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	write_config(data->settings);
}

void set_green(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	data->settings->green = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	write_config(data->settings);
}

void set_blue(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	data->settings->blue = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	write_config(data->settings);
}

void set_alpha(GtkWidget* widget, gpointer d) {
	AppData* data = (AppData*)d;
	data->settings->alpha = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	write_config(data->settings);
}

void close_dav(GtkWidget* window, gpointer d) {
	AppData* data = (AppData*)d;

	if (data->notebook) {
		int num_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(data->notebook));
		for (int i = 0; i < num_pages; i++) {
			GtkWidget* page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(data->notebook), i);
			if (page) {
				gulong handler_id = g_signal_handler_find(page, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, on_gif_tab_destroyed, NULL);
				if (handler_id > 0) {
					g_signal_handler_disconnect(page, handler_id);
				}
			}
		}
	}

	if (data->gifs) {
		GList* iter = data->gifs;
		while (iter) {
			Gif* gif = (Gif*)iter->data;
			if (gif) {
				gif_free(gif);
			}
			iter = g_list_next(iter);
		}
		g_list_free(data->gifs);
		data->gifs = NULL;
	}

	if(data->overlay != NULL) {
		gtk_widget_destroy(data->overlay);
		data->overlay = NULL;
	}

	close_stream(data->stream);

	if (data->input_region) {
		cairo_region_destroy(data->input_region);
		data->input_region = NULL;
	}

	if (data->settings) {
		free(data->settings->path);
		data->settings->path = NULL;
		free(data->settings);
		data->settings = NULL;
	}

	if (data->stream) {
		free(data->stream);
		data->stream = NULL;
	}

#if defined(__APPLE__)
	if (data->native_window) {
		macos_destroy_settings_window(data->native_window);
		data->native_window = NULL;
	}
	gtk_main_quit();
#endif
}

gboolean on_window_delete(GtkWidget *window, GdkEvent *event, AppData *data) {
	gtk_widget_hide(window);
	return TRUE;
}

#if defined(__APPLE__)
static gboolean on_window_focus_in(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	// Force window activation on macOS to prevent unresponsive UI
	gtk_window_present(GTK_WINDOW(widget));
	return FALSE;
}

static gboolean on_button_press(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	// Ensure window stays active after clicks on macOS
	GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
	if (GTK_IS_WINDOW(toplevel)) {
		gtk_window_present(GTK_WINDOW(toplevel));
	}
	return FALSE;
}
#endif

static void on_load_gifs_clicked(GtkButton* button, AppData* data) {
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
		"Open GIF list",
		GTK_WINDOW(data->window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Open", GTK_RESPONSE_ACCEPT,
		NULL
		);

	GtkFileFilter* filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "UWU files");
	gtk_file_filter_add_pattern(filter, "*.uwu");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		load_gifs(filename, data);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

static void on_save_gifs_clicked(GtkButton* button, AppData* data) {
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
		"Save GIF list",
		GTK_WINDOW(data->window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Save", GTK_RESPONSE_ACCEPT,
		NULL
		);

	GtkFileFilter* filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "UWU files");
	gtk_file_filter_add_pattern(filter, "*.uwu");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		const char* ext = strrchr(filename, '.');
		char* final_name = NULL;
		if (!ext || g_ascii_strcasecmp(ext, ".uwu") != 0) {
			final_name = g_strconcat(filename, ".uwu", NULL);
		} else {
			final_name = g_strdup(filename);
		}

		save_gifs(final_name, data);
		g_free(final_name);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}


void create_window(AppData* data) {
	data->overlay = NULL;
	data->input_region = cairo_region_create();
	data->stream->speed = data->settings->speed;

#if defined(__APPLE__)
	// Use native macOS window instead of GTK
	data->native_window = macos_create_settings_window(data);
	macos_show_settings_window(data->native_window);
	return;
#endif

	gtk_window_set_default_size(GTK_WINDOW(data->window), 600, 500);

	data->header = gtk_header_bar_new();
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(data->header), TRUE);
	gtk_header_bar_set_title(GTK_HEADER_BAR(data->header), "Directional Audio Visualizer");
	gtk_window_set_titlebar(GTK_WINDOW(data->window), data->header);

	GtkWidget* add_gif_btn = gtk_button_new_with_label("+GIF");
	gtk_header_bar_pack_end(GTK_HEADER_BAR(data->header), add_gif_btn);
	g_signal_connect(add_gif_btn, "clicked", G_CALLBACK(on_add_gif_clicked), data);

	GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_container_set_border_width(GTK_CONTAINER(outer_box), 10);

	data->notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(data->notebook), TRUE);
	gtk_box_pack_start(GTK_BOX(outer_box), data->notebook, TRUE, TRUE, 0);

	GtkWidget* settings_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_container_set_border_width(GTK_CONTAINER(settings_tab), 10);

	data->device_name = gtk_label_new("");
	gtk_widget_set_halign(data->device_name, GTK_ALIGN_CENTER);
	gtk_box_pack_start(GTK_BOX(settings_tab), data->device_name, FALSE, FALSE, 0);

	GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_container_set_border_width(GTK_CONTAINER(main_box), 10);
	gtk_box_pack_start(GTK_BOX(settings_tab), main_box, TRUE, TRUE, 0);

	GtkWidget* settings_frame = gtk_frame_new("Settings");
	gtk_widget_set_hexpand(settings_frame, TRUE);
	gtk_container_add(GTK_CONTAINER(main_box), settings_frame);

	GtkWidget* settings_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(settings_grid), 8);
	gtk_grid_set_row_spacing(GTK_GRID(settings_grid), 6);
	gtk_widget_set_margin_start(settings_grid, 12);
	gtk_widget_set_margin_end(settings_grid, 12);
	gtk_widget_set_margin_top(settings_grid, 12);
	gtk_widget_set_margin_bottom(settings_grid, 12);
	gtk_widget_set_hexpand(settings_grid, TRUE);
	gtk_container_add(GTK_CONTAINER(settings_frame), settings_grid);

	GtkWidget* controls_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	gtk_widget_set_halign(controls_box, GTK_ALIGN_START);
	gtk_widget_set_valign(controls_box, GTK_ALIGN_START);
	gtk_widget_set_hexpand(controls_box, FALSE);
	gtk_box_pack_start(GTK_BOX(main_box), controls_box, FALSE, FALSE, 6);

	data->devices = gtk_combo_box_text_new();
	g_signal_connect(G_OBJECT(data->devices), "changed", G_CALLBACK(select_device), data);
	refresh(NULL, data);

	data->start_stop = gtk_button_new_with_label("Start");
	GtkWidget* button2 = gtk_button_new_with_label("Refresh");
	g_signal_connect(G_OBJECT(data->start_stop), "clicked", G_CALLBACK(start_stop), data);
	g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(refresh), data);

	gtk_box_pack_start(GTK_BOX(controls_box), data->devices, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(controls_box), data->start_stop, FALSE, FALSE, 6);
	gtk_box_pack_start(GTK_BOX(controls_box), button2, FALSE, FALSE, 0);

	int row = 0;

	GtkWidget* radius = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1000, 1);
	gtk_range_set_value(GTK_RANGE(radius), data->settings->radius);
	g_signal_connect(radius, "value-changed", G_CALLBACK(set_radius), data);
	gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("Radius"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(settings_grid), radius, 1, row++, 1, 1);

	GtkWidget* space = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1000, 1);
	gtk_range_set_value(GTK_RANGE(space), data->settings->space);
	g_signal_connect(space, "value-changed", G_CALLBACK(set_space), data);
	gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("Distance"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(settings_grid), space, 1, row++, 1, 1);

	GtkWidget* speed = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1000, 1);
	gtk_range_set_value(GTK_RANGE(speed), data->stream->speed);
	g_signal_connect(speed, "value-changed", G_CALLBACK(set_speed), data);
	gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("Speed"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(settings_grid), speed, 1, row++, 1, 1);

	GtkWidget* red = gtk_spin_button_new_with_range(0, 1.0, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(red), data->settings->red);
	g_signal_connect(red, "value-changed", G_CALLBACK(set_red), data);
	gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("Red"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(settings_grid), red, 1, row++, 1, 1);

	GtkWidget* green = gtk_spin_button_new_with_range(0, 1.0, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(green), data->settings->green);
	g_signal_connect(green, "value-changed", G_CALLBACK(set_green), data);
	gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("Green"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(settings_grid), green, 1, row++, 1, 1);

	GtkWidget* blue = gtk_spin_button_new_with_range(0, 1.0, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(blue), data->settings->blue);
	g_signal_connect(blue, "value-changed", G_CALLBACK(set_blue), data);
	gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("Blue"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(settings_grid), blue, 1, row++, 1, 1);

	GtkWidget* alpha = gtk_spin_button_new_with_range(0, 1.0, 0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(alpha), data->settings->alpha);
	g_signal_connect(alpha, "value-changed", G_CALLBACK(set_alpha), data);
	gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("Alpha"), 0, row, 1, 1);
	gtk_grid_attach(GTK_GRID(settings_grid), alpha, 1, row++, 1, 1);


	GtkWidget* load_btn = gtk_button_new_with_label("Load GIFs");
	GtkWidget* save_btn = gtk_button_new_with_label("Save GIFs");

	g_signal_connect(load_btn, "clicked", G_CALLBACK(on_load_gifs_clicked), data);
	g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_gifs_clicked), data);

	gtk_box_pack_start(GTK_BOX(controls_box), load_btn, FALSE, FALSE, 6);
	gtk_box_pack_start(GTK_BOX(controls_box), save_btn, FALSE, FALSE, 6);

	gtk_notebook_append_page(GTK_NOTEBOOK(data->notebook), settings_tab, gtk_label_new("Visualizer"));

	gtk_container_add(GTK_CONTAINER(data->window), outer_box);
	gtk_widget_show_all(data->window);

#if defined(__APPLE__)
	// Connect focus and button handlers to keep window responsive on macOS
	g_signal_connect(data->window, "focus-in-event", G_CALLBACK(on_window_focus_in), NULL);
	gtk_widget_add_events(data->window, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(data->window, "button-press-event", G_CALLBACK(on_button_press), NULL);
	
	// Force initial activation
	gtk_window_present(GTK_WINDOW(data->window));
#endif
}
