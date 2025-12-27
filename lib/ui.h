#ifndef UI_H
#define UI_H

#ifdef __APPLE__
void* ui_create_settings_window(void* app_data);
void ui_show_settings_window(void* window);
void ui_destroy_settings_window(void* window);
void* ui_create_overlay(void* app_data);
void ui_close_overlay(void* overlay);
void ui_start_app_loop();
#endif

#endif
