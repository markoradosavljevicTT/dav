#ifndef GIF_H
#define GIF_H

#include "data.h"

Gif* gif_new(const char* path);
void gif_free(Gif* g);
void add_gif_tab(AppData* data, const char* path);

void load_gifs(const char* path, AppData* data);
void save_gifs(const char* path, AppData* data);

#endif
