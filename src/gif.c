#include "gif.h"
#include "data.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <ImageIO/ImageIO.h>
#endif

Gif* gif_new(const char* path) {
	Gif* g = malloc(sizeof(Gif));
	if (!g) return NULL;

	g->path = strdup(path);
	if (!g->path) {
		free(g);
		return NULL;
	}

	g->gif_x = 0;
	g->gif_y = 0;
	g->gif_width = 200;
	g->gif_height = 200;
	g->imageSource = NULL;
	g->currentFrame = 0;
	g->frameCount = 0;
	g->frameDurations = NULL;
	g->lastFrameTime = 0.0;
	return g;
}

void gif_free(Gif* g) {
	if (!g) return;
	free(g->path);
	if (g->frameDurations) free(g->frameDurations);
#ifdef __APPLE__
	if (g->imageSource) {
		CFRelease((CGImageSourceRef)g->imageSource);
	}
#endif
	free(g);
}

void add_gif_tab(AppData* data, const char* path) {
	if (!data || !path) return;
	
	Gif* gif = gif_new(path);
	if (!gif) return;
	
	GifList* node = malloc(sizeof(GifList));
	if (!node) {
		gif_free(gif);
		return;
	}
	node->gif = gif;
	node->next = NULL;
	
	if (!data->gifs) {
		data->gifs = node;
	} else {
		GifList* last = data->gifs;
		while (last->next) {
			last = last->next;
		}
		last->next = node;
	}
}

void save_gifs(const char* path, AppData* data) {
	if (!path || !data || !data->gifs) return;

	FILE* f = fopen(path, "w");
	if (!f) return;

	for (GifList* l = data->gifs; l != NULL; l = l->next) {
		Gif* g = l->gif;
		if (!g || !g->path) continue;
		fprintf(f, "%s %d %d %d %d\n", g->path, g->gif_x, g->gif_y, g->gif_width, g->gif_height);
	}

	fclose(f);
}

void load_gifs(const char* path, AppData* data) {
	if (!path || !data) return;

	FILE* f = fopen(path, "r");
	if (!f) return;

	char gif_path[1024];
	int x, y, w, h;

	while (fscanf(f, "%1023s %d %d %d %d", gif_path, &x, &y, &w, &h) == 5) {
		Gif* g = gif_new(gif_path);
		if (!g) continue;
		g->gif_x = x;
		g->gif_y = y;
		g->gif_width = w;
		g->gif_height = h;
		
		GifList* node = malloc(sizeof(GifList));
		if (!node) {
			gif_free(g);
			continue;
		}
		node->gif = g;
		node->next = NULL;
		
		if (!data->gifs) {
			data->gifs = node;
		} else {
			GifList* last = data->gifs;
			while (last->next) {
				last = last->next;
			}
			last->next = node;
		}
	}

	fclose(f);
}
