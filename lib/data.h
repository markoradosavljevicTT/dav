#ifndef DATA_H
#define DATA_H

#include <stdbool.h>

typedef struct {
	char* path;
	int gif_x;
	int gif_y;
	int gif_width;
	int gif_height;
	void* imageSource; // CGImageSourceRef for animation
	int currentFrame;
	int frameCount;
	double* frameDurations; // Duration for each frame
	double lastFrameTime; // Time when last frame was displayed
} Gif;

typedef struct GifList {
	Gif* gif;
	struct GifList* next;
} GifList;

typedef struct {
	void* overlay; // Native overlay window
	void* native_window; // Native settings window
	GifList* gifs;
} AppData;

#endif
