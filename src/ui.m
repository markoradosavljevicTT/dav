#import <Cocoa/Cocoa.h>
#import <ImageIO/ImageIO.h>
#import <QuartzCore/QuartzCore.h>
#include "data.h"
#include "gif.h"

@class SettingsWindowController;
@class OverlayWindowController;

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, assign) SettingsWindowController* settingsController;
@property (nonatomic, assign) OverlayWindowController* overlayController;
@property (nonatomic, assign) AppData* appData;
@end

@implementation AppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    if (self.overlayController) {
        [[self.overlayController window] close];
    }
    if (self.settingsController) {
        [[self.settingsController window] close];
    }
    [NSApp terminate:nil];
    return NSTerminateNow;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag {
    if (self.settingsController) {
        NSWindow *window = [self.settingsController window];
        [window setIsVisible:YES];
        [window makeKeyAndOrderFront:nil];
        [sender activateIgnoringOtherApps:YES];
    }
    return YES;
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    if (self.settingsController) {
        NSWindow *window = [self.settingsController window];
        [window setIsVisible:YES];
        [window makeKeyAndOrderFront:nil];
        [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    }
}

- (void)showSettings:(id)sender {
    if (self.settingsController) {
        NSWindow *window = [self.settingsController window];
        if (window) {
            [window makeKeyAndOrderFront:nil];
            [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
        }
    }
}
@end

static AppDelegate *appDelegate = nil;

@interface SettingsWindowController : NSWindowController <NSWindowDelegate>
@property (nonatomic, assign) AppData* appData;
@property (nonatomic, strong) NSTabView* gifsTabView;
- (void)refreshGifTabs;
- (void)addGifTab:(Gif*)gif;
@end

@implementation SettingsWindowController

- (instancetype)initWithAppData:(AppData*)data {
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 700, 550)
                                                    styleMask:(NSWindowStyleMaskTitled | 
                                                              NSWindowStyleMaskClosable | 
                                                              NSWindowStyleMaskMiniaturizable |
                                                              NSWindowStyleMaskResizable)
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
    
    self = [super initWithWindow:window];
    if (self) {
        self.appData = data;
        [window setTitle:@"Directional Anime Visualizer"];
        [window setMinSize:NSMakeSize(500, 400)];
        [window setDelegate:self];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(handleAppReopen:)
                                                     name:NSApplicationDidBecomeActiveNotification
                                                   object:nil];
        
        [self setupUI];
    }
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)handleAppReopen:(NSNotification *)notification {
    NSWindow *window = [self window];
    if (window && ![window isVisible]) {
        [window setIsVisible:YES];
        [window makeKeyAndOrderFront:nil];
        [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    }
}

- (void)setupUI {
    NSView *contentView = self.window.contentView;
    
    CGFloat windowHeight = 550;
    CGFloat windowWidth = 700;
    CGFloat buttonY = windowHeight - 40;
    CGFloat buttonSpacing = 10;
    CGFloat buttonWidth = 100;
    CGFloat totalButtonWidth = buttonWidth * 3 + buttonSpacing * 2;
    
    NSView *buttonContainer = [[NSView alloc] initWithFrame:NSMakeRect((windowWidth - totalButtonWidth) / 2, buttonY, totalButtonWidth, 32)];
    [buttonContainer setAutoresizingMask:NSViewMinXMargin | NSViewMaxXMargin | NSViewMinYMargin];
    [contentView addSubview:buttonContainer];
    
    NSButton *addGifBtn = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, buttonWidth, 32)];
    [addGifBtn setTitle:@"Add GIF"];
    [addGifBtn setBezelStyle:NSBezelStyleRounded];
    [addGifBtn setTarget:self];
    [addGifBtn setAction:@selector(addGifButtonClicked:)];
    [buttonContainer addSubview:addGifBtn];
    
    NSButton *loadBtn = [[NSButton alloc] initWithFrame:NSMakeRect(buttonWidth + buttonSpacing, 0, buttonWidth, 32)];
    [loadBtn setTitle:@"Load GIFs"];
    [loadBtn setBezelStyle:NSBezelStyleRounded];
    [loadBtn setTarget:self];
    [loadBtn setAction:@selector(loadGifsButtonClicked:)];
    [buttonContainer addSubview:loadBtn];
    
    NSButton *saveBtn = [[NSButton alloc] initWithFrame:NSMakeRect((buttonWidth + buttonSpacing) * 2, 0, buttonWidth, 32)];
    [saveBtn setTitle:@"Save GIFs"];
    [saveBtn setBezelStyle:NSBezelStyleRounded];
    [saveBtn setTarget:self];
    [saveBtn setAction:@selector(saveGifsButtonClicked:)];
    [buttonContainer addSubview:saveBtn];
    
    self.gifsTabView = [[NSTabView alloc] initWithFrame:NSMakeRect(10, 10, 680, buttonY - 20)];
    [self.gifsTabView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [contentView addSubview:self.gifsTabView];
    
    [self refreshGifTabs];
}

- (void)addGifButtonClicked:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
    [panel setAllowedFileTypes:@[@"gif"]];
    
    if ([panel runModal] == NSModalResponseOK) {
        NSURL *url = [[panel URLs] firstObject];
        if (url) {
            const char *path = [[url path] UTF8String];
            add_gif_tab(self.appData, (char*)path);
            [self refreshGifTabs];
        }
    }
}

- (void)loadGifsButtonClicked:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];
    [panel setAllowedFileTypes:@[@"uwu"]];
    
    if ([panel runModal] == NSModalResponseOK) {
        NSURL *url = [[panel URLs] firstObject];
        if (url) {
            const char *path = [[url path] UTF8String];
            load_gifs((char*)path, self.appData);
            [self refreshGifTabs];
        }
    }
}

- (void)saveGifsButtonClicked:(id)sender {
    NSSavePanel *panel = [NSSavePanel savePanel];
    [panel setAllowedFileTypes:@[@"uwu"]];
    [panel setNameFieldStringValue:@"gifs.uwu"];
    
    if ([panel runModal] == NSModalResponseOK) {
        NSURL *url = [panel URL];
        if (url) {
            const char *path = [[url path] UTF8String];
            save_gifs((char*)path, self.appData);
        }
    }
}

- (void)refreshGifTabs {
    while ([self.gifsTabView numberOfTabViewItems] > 0) {
        [self.gifsTabView removeTabViewItem:[self.gifsTabView tabViewItemAtIndex:0]];
    }
    
    if (self.appData && self.appData->gifs) {
        for (GifList* l = self.appData->gifs; l != NULL; l = l->next) {
            Gif* gif = l->gif;
            if (gif) {
                [self addGifTab:gif];
            }
        }
    }
}

- (void)addGifTab:(Gif*)gif {
    if (!gif) return;
    
    if (gif->gif_width == 200 && gif->gif_height == 200) {
        NSString *gifPath = [NSString stringWithUTF8String:gif->path];
        NSURL *url = [NSURL fileURLWithPath:gifPath];
        CGImageSourceRef source = CGImageSourceCreateWithURL((__bridge CFURLRef)url, NULL);
        if (source) {
            if (CGImageSourceGetCount(source) > 0) {
                CGImageRef firstFrame = CGImageSourceCreateImageAtIndex(source, 0, NULL);
                if (firstFrame) {
                    gif->gif_width = (int)CGImageGetWidth(firstFrame);
                    gif->gif_height = (int)CGImageGetHeight(firstFrame);
                    CFRelease(firstFrame);
                }
            }
            CFRelease(source);
        }
    }
    
    NSView *gifView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 640, 400)];
    [gifView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    
    CGFloat viewHeight = [gifView frame].size.height;
    CGFloat yPos = viewHeight - 70;
    
    NSTextField *xLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, yPos, 100, 24)];
    [xLabel setStringValue:@"X Position:"];
    [xLabel setBezeled:NO];
    [xLabel setDrawsBackground:NO];
    [xLabel setEditable:NO];
    [xLabel setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:xLabel];
    
    NSTextField *xField = [[NSTextField alloc] initWithFrame:NSMakeRect(130, yPos, 100, 24)];
    [xField setIntValue:gif->gif_x];
    [xField setTag:(NSInteger)gif];
    [xField setTarget:self];
    [xField setAction:@selector(gifXChanged:)];
    [xField setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:xField];
    yPos -= 40;
    
    NSTextField *yLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, yPos, 100, 24)];
    [yLabel setStringValue:@"Y Position:"];
    [yLabel setBezeled:NO];
    [yLabel setDrawsBackground:NO];
    [yLabel setEditable:NO];
    [yLabel setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:yLabel];
    
    NSTextField *yField = [[NSTextField alloc] initWithFrame:NSMakeRect(130, yPos, 100, 24)];
    [yField setIntValue:gif->gif_y];
    [yField setTag:(NSInteger)gif];
    [yField setTarget:self];
    [yField setAction:@selector(gifYChanged:)];
    [yField setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:yField];
    yPos -= 40;
    
    NSTextField *wLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, yPos, 100, 24)];
    [wLabel setStringValue:@"Width:"];
    [wLabel setBezeled:NO];
    [wLabel setDrawsBackground:NO];
    [wLabel setEditable:NO];
    [wLabel setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:wLabel];
    
    NSTextField *wField = [[NSTextField alloc] initWithFrame:NSMakeRect(130, yPos, 100, 24)];
    [wField setIntValue:gif->gif_width];
    [wField setTag:(NSInteger)gif];
    [wField setTarget:self];
    [wField setAction:@selector(gifWidthChanged:)];
    [wField setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:wField];
    yPos -= 40;
    
    NSTextField *hLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, yPos, 100, 24)];
    [hLabel setStringValue:@"Height:"];
    [hLabel setBezeled:NO];
    [hLabel setDrawsBackground:NO];
    [hLabel setEditable:NO];
    [hLabel setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:hLabel];
    
    NSTextField *hField = [[NSTextField alloc] initWithFrame:NSMakeRect(130, yPos, 100, 24)];
    [hField setIntValue:gif->gif_height];
    [hField setTag:(NSInteger)gif];
    [hField setTarget:self];
    [hField setAction:@selector(gifHeightChanged:)];
    [hField setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:hField];
    yPos -= 60;
    
    NSButton *removeBtn = [[NSButton alloc] initWithFrame:NSMakeRect(20, yPos, 100, 32)];
    [removeBtn setTitle:@"Remove GIF"];
    [removeBtn setBezelStyle:NSBezelStyleRounded];
    [removeBtn setTag:(NSInteger)gif];
    [removeBtn setTarget:self];
    [removeBtn setAction:@selector(removeGifButtonClicked:)];
    [removeBtn setAutoresizingMask:NSViewMinYMargin];
    [gifView addSubview:removeBtn];
    
    NSImageView *imageView = [[NSImageView alloc] initWithFrame:NSMakeRect(280, 50, 340, 300)];
    [imageView setImageScaling:NSImageScaleProportionallyUpOrDown];
    [imageView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable | NSViewMinXMargin];
    
    NSString *gifPath = [NSString stringWithUTF8String:gif->path];
    NSImage *gifImage = [[NSImage alloc] initWithContentsOfFile:gifPath];
    if (gifImage) {
        [imageView setImage:gifImage];
        [imageView setAnimates:YES];
    }
    
    [gifView addSubview:imageView];
    
    NSString *tabTitle = [[NSString stringWithUTF8String:gif->path] lastPathComponent];
    NSTabViewItem *gifItem = [[NSTabViewItem alloc] initWithIdentifier:[NSValue valueWithPointer:gif]];
    [gifItem setLabel:tabTitle];
    [gifItem setView:gifView];
    [self.gifsTabView addTabViewItem:gifItem];
}

- (void)gifXChanged:(id)sender {
    NSTextField *field = (NSTextField*)sender;
    Gif *gif = (Gif*)[field tag];
    if (gif) {
        gif->gif_x = [field intValue];
    }
}

- (void)gifYChanged:(id)sender {
    NSTextField *field = (NSTextField*)sender;
    Gif *gif = (Gif*)[field tag];
    if (gif) {
        gif->gif_y = [field intValue];
    }
}

- (void)gifWidthChanged:(id)sender {
    NSTextField *field = (NSTextField*)sender;
    Gif *gif = (Gif*)[field tag];
    if (gif) {
        gif->gif_width = [field intValue];
    }
}

- (void)gifHeightChanged:(id)sender {
    NSTextField *field = (NSTextField*)sender;
    Gif *gif = (Gif*)[field tag];
    if (gif) {
        gif->gif_height = [field intValue];
    }
}

- (void)removeGifButtonClicked:(id)sender {
    NSButton *button = (NSButton*)sender;
    Gif *gif = (Gif*)[button tag];
    
    if (gif && self.appData) {
        if (self.appData->gifs && self.appData->gifs->gif == gif) {
            GifList* temp = self.appData->gifs;
            self.appData->gifs = self.appData->gifs->next;
            free(temp);
        } else {
            for (GifList* l = self.appData->gifs; l && l->next; l = l->next) {
                if (l->next->gif == gif) {
                    GifList* temp = l->next;
                    l->next = l->next->next;
                    free(temp);
                    break;
                }
            }
        }
        
        gif_free(gif);
        
        [self refreshGifTabs];
    }
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    [sender orderOut:nil];
    return NO;
}

@end

void* ui_create_settings_window(AppData* data) {
    NSApplication *app = [NSApplication sharedApplication];
    
    if (!appDelegate) {
        appDelegate = [[AppDelegate alloc] init];
        [app setDelegate:appDelegate];
        
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        [app activateIgnoringOtherApps:YES];
    }
    
    SettingsWindowController *controller = [[SettingsWindowController alloc] initWithAppData:data];
    appDelegate.settingsController = controller;
    appDelegate.appData = data;
    
    return (void*)controller;
}

void ui_show_settings_window(void* window) {
    SettingsWindowController *controller = (__bridge SettingsWindowController*)window;
    [[controller window] makeKeyAndOrderFront:nil];
    [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
}

void ui_close_settings_window(void* window) {
    SettingsWindowController *controller = (__bridge SettingsWindowController*)window;
    [[controller window] close];
}

void ui_destroy_settings_window(void* window) {
    SettingsWindowController *controller = (SettingsWindowController*)window;
    [[controller window] close];
    [controller release];
    controller = nil;
}

@interface OverlayView : NSView
@property (nonatomic, assign) AppData* appData;
@end

@implementation OverlayView

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    return self;
}

- (CGImageSourceRef)getImageSourceForGif:(Gif*)gif {
    CGImageSourceRef source = (CGImageSourceRef)gif->imageSource;
    
    if (!source) {
        NSString *path = [NSString stringWithUTF8String:gif->path];
        NSURL *url = [NSURL fileURLWithPath:path];
        source = CGImageSourceCreateWithURL((__bridge CFURLRef)url, NULL);
        if (source) {
            gif->imageSource = (void*)source;
            
            gif->frameCount = (int)CGImageSourceGetCount(source);
            gif->currentFrame = 0;
            
            if (gif->gif_width == 200 && gif->gif_height == 200 && gif->frameCount > 0) {
                CGImageRef firstFrame = CGImageSourceCreateImageAtIndex(source, 0, NULL);
                if (firstFrame) {
                    gif->gif_width = (int)CGImageGetWidth(firstFrame);
                    gif->gif_height = (int)CGImageGetHeight(firstFrame);
                    CFRelease(firstFrame);
                }
            }
            
            if (!gif->frameDurations && gif->frameCount > 0) {
                gif->frameDurations = malloc(sizeof(double) * gif->frameCount);
                for (int i = 0; i < gif->frameCount; i++) {
                    NSDictionary *frameProps = (__bridge NSDictionary*)CGImageSourceCopyPropertiesAtIndex(source, i, NULL);
                    NSDictionary *gifProps = frameProps[(NSString*)kCGImagePropertyGIFDictionary];
                    NSNumber *delayTime = gifProps[(NSString*)kCGImagePropertyGIFUnclampedDelayTime];
                    if (!delayTime) {
                        delayTime = gifProps[(NSString*)kCGImagePropertyGIFDelayTime];
                    }
                    gif->frameDurations[i] = delayTime ? [delayTime doubleValue] : 0.1;
                    CFRelease((__bridge CFTypeRef)frameProps);
                }
            }
        }
    }
    
    return source;
}

- (void)drawRect:(NSRect)dirtyRect {
    [[NSColor clearColor] set];
    NSRectFill(dirtyRect);
    
    if (!self.appData || !self.appData->gifs) return;
    
    for (GifList* l = self.appData->gifs; l != NULL; l = l->next) {
        Gif* gif = l->gif;
        if (!gif || !gif->path) continue;
        
        CGImageSourceRef imageSource = [self getImageSourceForGif:gif];
        if (!imageSource || gif->frameCount == 0) continue;
        
        CGImageRef frameImage = CGImageSourceCreateImageAtIndex(imageSource, gif->currentFrame, NULL);
        if (!frameImage) continue;
        
        CGFloat width = gif->gif_width > 0 ? gif->gif_width : CGImageGetWidth(frameImage);
        CGFloat height = gif->gif_height > 0 ? gif->gif_height : CGImageGetHeight(frameImage);
        CGFloat screenHeight = [[NSScreen mainScreen] frame].size.height;
        CGFloat flippedY = screenHeight - gif->gif_y - height;
        NSRect destRect = NSMakeRect(gif->gif_x, flippedY, width, height);
        
        NSImage *frameNSImage = [[NSImage alloc] initWithCGImage:frameImage size:NSZeroSize];
        [frameNSImage drawInRect:destRect];
        
        CGImageRelease(frameImage);
    }
}

@end

@interface OverlayWindowController : NSWindowController
@property (nonatomic, assign) AppData* appData;
@property (nonatomic, strong) NSTimer* animationTimer;
@end

@implementation OverlayWindowController

- (instancetype)initWithAppData:(AppData*)data {
    NSRect screenFrame = [[NSScreen mainScreen] frame];
    
    NSWindow *window = [[NSWindow alloc] initWithContentRect:screenFrame
                                                    styleMask:NSWindowStyleMaskBorderless
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
    
    self = [super initWithWindow:window];
    if (self) {
        self.appData = data;
        
        [window setOpaque:NO];
        [window setBackgroundColor:[NSColor clearColor]];
        [window setLevel:NSStatusWindowLevel];
        [window setIgnoresMouseEvents:YES];
        [window setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces |
                                       NSWindowCollectionBehaviorStationary |
                                       NSWindowCollectionBehaviorFullScreenAuxiliary];
        
        OverlayView *overlayView = [[OverlayView alloc] initWithFrame:screenFrame];
        overlayView.appData = data;
        [window setContentView:overlayView];
        
        self.animationTimer = [NSTimer timerWithTimeInterval:1.0/30.0
                                                      target:self
                                                    selector:@selector(advanceFrames)
                                                    userInfo:nil
                                                     repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:self.animationTimer forMode:NSRunLoopCommonModes];
        
        [window makeKeyAndOrderFront:nil];
    }
    return self;
}

- (void)advanceFrames {
    if (!self.appData || !self.appData->gifs) return;
    
    double currentTime = CACurrentMediaTime();
    
    for (GifList* l = self.appData->gifs; l != NULL; l = l->next) {
        Gif* gif = l->gif;
        if (!gif || gif->frameCount == 0 || !gif->frameDurations) continue;
        
        if (gif->lastFrameTime == 0.0) {
            gif->lastFrameTime = currentTime;
        }
        
        double frameDuration = gif->frameDurations[gif->currentFrame];
        if (currentTime - gif->lastFrameTime >= frameDuration) {
            gif->currentFrame = (gif->currentFrame + 1) % gif->frameCount;
            gif->lastFrameTime = currentTime;
        }
    }
    
    [[self.window contentView] setNeedsDisplay:YES];
}

- (void)dealloc {
    [self.animationTimer invalidate];
    self.animationTimer = nil;
    [super dealloc];
}

@end

void* ui_create_overlay(void* app_data) {
    AppData* data = (AppData*)app_data;
    OverlayWindowController *controller = [[OverlayWindowController alloc] initWithAppData:data];
    return (void*)controller;
}

void ui_close_overlay(void* overlay) {
    if (!overlay) return;
    OverlayWindowController *controller = (OverlayWindowController*)overlay;
    [[controller window] close];
    [controller release];
    controller = nil;
}

void ui_start_app_loop() {
    [NSApp run];
}
