CC = gcc
MKDIR = mkdir -p
RM = rm -rf
PKG = pkg-config

SRC = src
LIB = lib
BUILD = build

SRCS = $(wildcard $(SRC)/*.c)
M_OBJC_SRCS = $(wildcard $(SRC)/*.m)
M_OBJS = $(SRCS:$(SRC)/%.c=$(BUILD)/mac/%.o) $(M_OBJC_SRCS:$(SRC)/%.m=$(BUILD)/mac/%.o)

FLAGS = -Wall -I$(LIB) -O3 -MD -MP -Wno-deprecated-declarations
M_CFLAGS = $(FLAGS) -D__APPLE__ $(shell $(PKG) --cflags gtk+-3.0) -I/opt/homebrew/include
M_LDFLAGS = $(shell $(PKG) --libs gtk+-3.0) -L/opt/homebrew/lib -lportaudio -lm -framework Cocoa -framework CoreFoundation

RES_XML = icon.xml
RES_GRES = glib-compile-resources

.PHONY: all run clean

all: $(BUILD)/mac/dav

$(BUILD)/mac/dav: $(M_OBJS) $(BUILD)/mac/resources.o
	$(CC) $^ -o $@ $(M_LDFLAGS)

$(BUILD)/mac/%.o: $(SRC)/%.c | $(BUILD)/mac
	$(CC) $(M_CFLAGS) -c $< -o $@

$(BUILD)/mac/%.o: $(SRC)/%.m | $(BUILD)/mac
	$(CC) $(M_CFLAGS) -c $< -o $@

$(BUILD)/mac:
	@$(MKDIR) $@

$(BUILD)/mac/resources.o: $(BUILD)/mac/resources.c
	$(CC) $(M_CFLAGS) -c $< -o $@

$(BUILD)/mac/resources.c: $(RES_XML) icon.png | $(BUILD)/mac
	$(RES_GRES) --target=$@ --generate-source --sourcedir=. $<

run: all
	./$(BUILD)/mac/dav

clean:
	@$(RM) $(BUILD)

-include $(BUILD)/mac/*.d
