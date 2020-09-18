#import "cocoa-app.h"
#import "utils.h"
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

/**
 * The delegates handle various events and behavior that come from the
 * application. In this case, this handles the behavior of the overall app.
 *
 * https://developer.apple.com/documentation/appkit/nsapplicationdelegate?language=objc
 *
 * - applicationWillFinishLaunching:
 * - applicationDidFinishLaunching:
 * - applicationWillBecomeActive:
 * - applicationDidBecomeActive:
 * - applicationWillResignActive:
 * - applicationDidResignActive:
 * - applicationShouldTerminate:
 * - applicationShouldTerminateAfterLastWindowClosed:
 * - applicationWillTerminate:
 * - applicationWillHide:
 * - applicationDidHide:
 * - applicationWillUnhide:
 * - applicationDidUnhide:
 * - applicationWillUpdate:
 * - applicationDidUpdate:
 * - applicationShouldHandleReopen:hasVisibleWindows:
 * - applicationDockMenu:
 * - application:willPresentError:
 * - applicationDidChangeScreenParameters:
 * - application:willContinueUserActivityWithType:
 * - application:continueUserActivity:restorationHandler:
 * - application:didFailToContinueUserActivityWithType:error:
 * - application:didUpdateUserActivity:
 * - application:didRegisterForRemoteNotificationsWithDeviceToken:
 * - application:didFailToRegisterForRemoteNotificationsWithError:
 * - application:didReceiveRemoteNotification:
 * - application:userDidAcceptCloudKitShareWithMetadata:
 * - application:openURLs:
 * - application:openFile:
 * - application:openFileWithoutUI:
 * - application:openTempFile:
 * - application:openFiles:
 * - applicationOpenUntitledFile:
 * - applicationShouldOpenUntitledFile:
 * - application:printFile:
 * - application:printFiles:withSettings:showPrintPanels:
 * - application:didDecodeRestorableState:
 * - application:willEncodeRestorableState:
 * - applicationDidChangeOcclusionState:
 * - application:delegateHandlesKey:
 */
@interface AppDelegate : NSObject<NSApplicationDelegate> {
}
@end

@implementation AppDelegate
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
  return YES;
}
@end

/**
 * The delegate handle various events and behavior that come from the window.
 *
 * https://developer.apple.com/documentation/appkit/nswindowdelegate?language=objc
 */
@interface WindowDelegate : NSObject<NSWindowDelegate>
@property (unsafe_unretained, assign, nonatomic) viz::Tick* tick;
@end

@implementation WindowDelegate
- (NSSize)windowWillResize:(NSWindow*)window toSize:(NSSize)frameSize
{
  _tick->width = window.backingScaleFactor * frameSize.width;
  _tick->height = window.backingScaleFactor * frameSize.height;
  return frameSize;
}
@end

/**
 * A specialized view that creates, configures, and displays Metal objects.
 *
 * https://developer.apple.com/documentation/metalkit/mtkview?language=objc
 * https://developer.apple.com/documentation/appkit/nsview?language=objc
 */
@interface View : MTKView

- (id)initWithFrame:(CGRect)frame
             tickFn:(viz::TickFn*)tickFn
               tick:(viz::Tick*)tick;
- (void)drawRect:(NSRect)rect;

/**
 * This is the C++ function for handling the draw loop. The loop is unowned.
 */
@property (unsafe_unretained, assign, nonatomic) viz::TickFn* tickFn;
@property (unsafe_unretained, assign, nonatomic) viz::Tick* tick;

@end

@implementation View {
}
//
- (id)initWithFrame:(CGRect)frame
             tickFn:(viz::TickFn*)tickFn
               tick:(viz::Tick*)tick
{
  self = [super initWithFrame:frame];

  _tickFn = tickFn;
  _tick = tick;

  // Setup the tracking area so that the mousemove events are used.
  NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
    initWithRect:self.frame
         options:NSTrackingActiveAlways | NSTrackingMouseMoved |
                 NSTrackingMouseEnteredAndExited
           owner:self
        userInfo:nil];

  [self addTrackingArea:trackingArea];

  return self;
}

/**
 * This method is called for every draw tick.
 *
 * See "Configuring the Drawing Behavior":
 * https://developer.apple.com/documentation/metalkit/mtkview?language=objc
 */
- (void)drawRect:(NSRect)rect
{

  if (self.currentDrawable == nil || self.currentRenderPassDescriptor == nil) {
    // Only draw this frame if these are available.
    return;
  }

  // Give the tick the information it needs about the current view.
  _tick->drawable = ns::Handle{ (__bridge void*)self.currentDrawable };
  _tick->renderPassDescriptor =
    ns::Handle{ (__bridge void*)self.currentRenderPassDescriptor };

  _tick->Update();

  (*_tickFn)(*_tick);
}

- (void)mouseDown:(NSEvent*)event
{
  _tick->isMouseDown = true;
}

- (void)mouseUp:(NSEvent*)event
{
  _tick->isMouseDown = false;
}

- (void)mouseMoved:(NSEvent*)event
{
  NSPoint position = [event locationInWindow];
  _tick->mouse = viz::Vec2<float>{
    static_cast<float>(position.x * self.window.backingScaleFactor),
    static_cast<float>(position.y * self.window.backingScaleFactor)
  };
}

- (void)mouseExited:(NSEvent*)event
{
  _tick->mouse = {};
}

@end

void
viz::Tick::Update()
{
  // Update timing information.
  auto previousTime = currentTime;
  auto currentTime = std::chrono::system_clock::now();

  if (hasRunOnce) {
    // Ensure the dt and milliseconds are implemented correctly
    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                     currentTime - startTime)
                     .count();
    seconds = milliseconds / 1000;
    dt = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime -
                                                               currentTime)
           .count();

    // The wall clock is not guaranteed to be monotonically increasing.
    dt = fmax(dt, 0);

    ++tick;
  }

  hasRunOnce = true;
}

void
viz::InitApp(const mtlpp::Device& device, viz::TickFn* tickFn)
{
  // The NSApplication is an object that manages an app’s main event loop and
  // resources used by all of that app’s objects.
  //
  // The shared NSApplication object performs the important task of receiving
  // events from the window server and distributing them to the proper
  // NSResponder objects
  //
  // https://developer.apple.com/documentation/appkit/nsapplication?language=objc
  NSApplication* app = [NSApplication sharedApplication];
  AppDelegate* appDelegate = [AppDelegate new];
  [app setDelegate:appDelegate];

  // The app is an ordinary app that appears in the Dock and may have
  // a user interface.
  [app setActivationPolicy:NSApplicationActivationPolicyRegular];

  // Set up the window and view.
  NSRect frame = NSMakeRect(0, 0, 800, 600);

  // The window that an app displays on the screen.
  // https://developer.apple.com/documentation/appkit/nswindow?language=objc
  NSWindow* window = [[NSWindow alloc]
    initWithContentRect:frame
              styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                         NSWindowStyleMaskResizable)
                backing:NSBackingStoreBuffered
                  defer:NO];

  [window setTitle:[NSString stringWithUTF8String:getExecutableName().c_str()]];

  // The tick is an object that is re-used on every frame draw call that
  // contains the current tick information.
  viz::Tick tick{ frame.size.width * window.backingScaleFactor,
                  frame.size.height * window.backingScaleFactor };

  // The delegate handles events related to the window.
  WindowDelegate* windowDelegate = [WindowDelegate new];
  [windowDelegate setTick:&tick];
  [window setDelegate:windowDelegate];
  [window setFrameAutosaveName:@"Main Window"];

  // The view that draws the contents of the window.
  MTKView* view = [[View alloc] initWithFrame:frame tickFn:tickFn tick:&tick];
  [window setContentView:view];
  view.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
  // Reset the depth buffer on every draw. 1.0 is furthest away, and 0 is
  // closest.
  view.clearDepth = 1.0;

  // Pass our metal device to the view.
  view.device = (__bridge id<MTLDevice>)device.GetPtr();

  // Show the window.
  [window makeKeyAndOrderFront:nil];

  // Setup the menu.
  NSMenu* mainMenu = [NSMenu new];
  NSMenuItem* appMenuItem = [NSMenuItem new];
  [mainMenu addItem:appMenuItem];

  NSMenu* appMenu = [NSMenu new];
  NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                        action:@selector(stop:)
                                                 keyEquivalent:@"q"];
  [appMenu addItem:quitMenuItem];
  [appMenuItem setSubmenu:appMenu];
  [app setMainMenu:mainMenu];

  [app run];
}
