#include "config.h"
#include <stdlib.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_config_path() {
	const char* xdgConfigHome = getenv("XDG_CONFIG_HOME");
	if (xdgConfigHome == NULL || strlen(xdgConfigHome) == 0) {
		const char* homeDir = getenv("HOME");
		if (homeDir == NULL || strlen(homeDir) == 0) {
			fprintf(stderr, "Error: HOME environment variable not set.\n");
			exit(EXIT_FAILURE);
		}
		return strcat(strcpy(malloc(strlen(homeDir) + strlen("/.config/dav/config") + 1), homeDir), "/.config/dav/config");
	}
	return strcat(strcpy(malloc(strlen(xdgConfigHome) + strlen("/dav/config") + 1), xdgConfigHome), "/dav/config");
}

void write_config(const Config* config) {
	FILE* file = fopen(config->path, "w");
	if (file == NULL) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}
	fprintf(file, "# Configuration file for Directional Audio Visualizer\n\n");
	fprintf(file, "# Radius\n");
	fprintf(file, "radius = %d\n\n", config->radius);
	fprintf(file, "# Space\n");
	fprintf(file, "space = %d\n\n", config->space);
	fprintf(file, "# Speed\n");
	fprintf(file, "speed = %d\n\n", config->speed);
	fprintf(file, "# Color\n");
	fprintf(file, "red = %lf\n\n", config->red);
	fprintf(file, "green = %lf\n\n", config->green);
	fprintf(file, "blue = %lf\n\n", config->blue);
	fprintf(file, "alpha = %lf\n\n", config->alpha);

	fclose(file);
}

Config* read_config() {
	Config* config = malloc(sizeof(Config));
	*config = (Config){
		.path = get_config_path(),
		.radius = -1,
		.space = -1,
		.speed = -1,
		.red = -1,
		.green = -1,
		.blue = -1,
		.alpha = -1,
	};

	Config default_config = {
		.path = NULL,
		.radius = 50,
		.space = 500,
		.speed = 200,
		.red = 0.0,
		.green = 1.0,
		.blue = 1.0,
		.alpha = 0.5,
	};

	char* config_dir = strdup(config->path);
	config_dir[strlen(config_dir) - strlen("/config")] = '\0';
	struct stat st = {0};
	if (stat(config_dir, &st) == -1) {
		if (mkdir(config_dir, 0700) == -1) {
			perror("Error creating config directory");
			exit(EXIT_FAILURE);
		}
	}

	FILE* file = fopen(config->path, "r");

	if (file == NULL) {
		default_config.path = config->path;
		*config = default_config;
	}
	else {
		char buffer[1000];
		while (fgets(buffer, sizeof(buffer), file) != NULL) {
			char key[1000];
			double value;

			if (sscanf(buffer, "%s = %lf", key, &value) == 2) {
				if (strcmp(key, "radius") == 0) {
					config->radius = value;
				} else if (strcmp(key, "space") == 0) {
					config->space = (int)value;
				} else if (strcmp(key, "speed") == 0) {
					config->speed = value;
				} else if (strcmp(key, "red") == 0) {
					config->red = value;
				} else if (strcmp(key, "green") == 0) {
					config->green = value;
				} else if (strcmp(key, "blue") == 0) {
					config->blue = value;
				} else if (strcmp(key, "alpha") == 0) {
					config->alpha = value;
				}
			}
		}
		if(config->radius < 0) config->radius = default_config.radius;
		if(config->space < 0) config->space = default_config.space;
		if(config->speed < 0) config->speed = default_config.speed;
		if(config->red < 0) config->red = default_config.red;
		if(config->green < 0) config->green = default_config.green;
		if(config->blue < 0) config->blue = default_config.blue;
		if(config->alpha < 0) config->alpha = default_config.alpha;

		fclose(file);
	}

	write_config(config);
	return config;
}
