# Directional Anime Visualizer - macOS Build System

# Build tools
CC = gcc
MKDIR = mkdir -p
RM = rm -rf

# Directory structure
SRC = src
LIB = lib
BUILD = build

# Source files
SRCS = $(wildcard $(SRC)/*.c)
OBJC_SRCS = $(wildcard $(SRC)/*.m)
OBJS = $(SRCS:$(SRC)/%.c=$(BUILD)/mac/%.o) $(OBJC_SRCS:$(SRC)/%.m=$(BUILD)/mac/%.o)

# Compiler flags
CFLAGS = -Wall -I$(LIB) -O3 -MD -MP -Wno-deprecated-declarations -D__APPLE__
LDFLAGS = -framework Cocoa -framework CoreFoundation -framework QuartzCore -framework ImageIO

# Application bundle
APP_NAME = DirectionalAnimeVisualizer
APP_BUNDLE = $(BUILD)/mac/$(APP_NAME).app
APP_CONTENTS = $(APP_BUNDLE)/Contents
APP_MACOS = $(APP_CONTENTS)/MacOS
APP_RESOURCES = $(APP_CONTENTS)/Resources

.PHONY: all build app install uninstall run clean

# Default target
all: app

# Build binary
build: $(BUILD)/mac/dav

$(BUILD)/mac/dav: $(OBJS)
	@echo "Linking executable..."
	@$(CC) $^ -o $@ $(LDFLAGS)

# Compile C source files
$(BUILD)/mac/%.o: $(SRC)/%.c | $(BUILD)/mac
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile Objective-C source files
$(BUILD)/mac/%.o: $(SRC)/%.m | $(BUILD)/mac
	@$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(BUILD)/mac:
	@$(MKDIR) $@

# Create .app bundle
app: build
	@echo "Creating app bundle..."
	@$(MKDIR) $(APP_MACOS) $(APP_RESOURCES)
	@cp $(BUILD)/mac/dav $(APP_MACOS)/
	@cp icon.png $(APP_RESOURCES)/
	@echo "Generating Info.plist..."
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $(APP_CONTENTS)/Info.plist
	@echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">' >> $(APP_CONTENTS)/Info.plist
	@echo '<plist version="1.0">' >> $(APP_CONTENTS)/Info.plist
	@echo '<dict>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleName</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>Directional Anime Visualizer</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleDisplayName</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>Directional Anime Visualizer</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleIdentifier</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>com.dav.directionalanimevisualizer</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleVersion</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>1.0</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundlePackageType</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>APPL</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleExecutable</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>dav</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>CFBundleIconFile</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>icon</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>LSMinimumSystemVersion</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <string>10.13</string>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <key>NSHighResolutionCapable</key>' >> $(APP_CONTENTS)/Info.plist
	@echo '    <true/>' >> $(APP_CONTENTS)/Info.plist
	@echo '</dict>' >> $(APP_CONTENTS)/Info.plist
	@echo '</plist>' >> $(APP_CONTENTS)/Info.plist
	@echo "Converting icon..."
	@$(MKDIR) $(BUILD)/icon.iconset
	@sips -z 16 16 icon.png --out $(BUILD)/icon.iconset/icon_16x16.png > /dev/null 2>&1
	@sips -z 32 32 icon.png --out $(BUILD)/icon.iconset/icon_16x16@2x.png > /dev/null 2>&1
	@sips -z 32 32 icon.png --out $(BUILD)/icon.iconset/icon_32x32.png > /dev/null 2>&1
	@sips -z 64 64 icon.png --out $(BUILD)/icon.iconset/icon_32x32@2x.png > /dev/null 2>&1
	@sips -z 128 128 icon.png --out $(BUILD)/icon.iconset/icon_128x128.png > /dev/null 2>&1
	@sips -z 256 256 icon.png --out $(BUILD)/icon.iconset/icon_128x128@2x.png > /dev/null 2>&1
	@sips -z 256 256 icon.png --out $(BUILD)/icon.iconset/icon_256x256.png > /dev/null 2>&1
	@sips -z 512 512 icon.png --out $(BUILD)/icon.iconset/icon_256x256@2x.png > /dev/null 2>&1
	@sips -z 512 512 icon.png --out $(BUILD)/icon.iconset/icon_512x512.png > /dev/null 2>&1
	@sips -z 1024 1024 icon.png --out $(BUILD)/icon.iconset/icon_512x512@2x.png > /dev/null 2>&1
	@iconutil -c icns $(BUILD)/icon.iconset -o $(APP_RESOURCES)/icon.icns
	@$(RM) $(BUILD)/icon.iconset
	@echo "✓ App bundle created at: $(APP_BUNDLE)"

# Install to /Applications
install: app
	@echo "Installing to /Applications..."
	@$(RM) "/Applications/$(APP_NAME).app"
	@cp -R "$(APP_BUNDLE)" /Applications/
	@echo "✓ Installation complete!"

# Uninstall from /Applications
uninstall:
	@echo "Uninstalling from /Applications..."
	@$(RM) "/Applications/$(APP_NAME).app"
	@echo "✓ Uninstallation complete!"

# Run without creating bundle
run: build
	@./$(BUILD)/mac/dav

# Clean build artifacts
clean:
	@$(RM) $(BUILD)
	@echo "✓ Build artifacts cleaned"

# Include auto-generated dependencies
-include $(BUILD)/mac/*.d
