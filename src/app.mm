#import "app.h"
#import <Cocoa/Cocoa.h>

/**
 * The delegates handle various events and behavior that come from the
 * application. In this case, this handles the behavior of the overall app.
 *
 * https://developer.apple.com/documentation/appkit/nsapplicationdelegate?language=objc
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
@interface WindowDelegate : NSObject<NSWindowDelegate> {
}
@end

@implementation WindowDelegate
- (void)windowDidResize:(NSNotification*)notification
{
  // Implement any resize logic here.
}
@end

/**
 * The view is in charge of drawing on the screen. It can handle specific mouse
 * events that happen inside the view. These can then be passed on to our app.
 *
 * https://developer.apple.com/documentation/appkit/nsview?language=objc
 */
@interface View : NSView {
}
- (void)drawRect:(NSRect)rect;
@end

@implementation View
//
- (id)initWithFrame:(CGRect)frame
{
  self = [super initWithFrame:frame];

  // Setup the tracking area so that the mousemove events are used.
  NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
    initWithRect:self.frame
         options:NSTrackingActiveAlways | NSTrackingMouseMoved
           owner:self
        userInfo:nil];

  [self addTrackingArea:trackingArea];

  return self;
}

/**
 * This is the logic for drawing the rectangle, it only gets called when needed.
 */
- (void)drawRect:(NSRect)rect
{
  // get the size of the app's window and view objects
  float width = [self bounds].size.width;
  float height = [self bounds].size.height;

  // Draw the outer box.
  [[NSColor colorWithCalibratedRed:0.2f green:0.2f blue:0.2f alpha:1.0f] set];
  NSRectFill([self bounds]);

  // Generate a random grey color, so that we know when this is updating.
  float color = (float)rand() / RAND_MAX;

  // Draw the inner box.
  [[NSColor colorWithCalibratedRed:color green:color blue:color
                             alpha:1.0f] set];
  NSRectFill(
    NSMakeRect(width * 0.25, height * 0.25, width * 0.5, height * 0.5));
}

- (void)mouseDown:(NSEvent*)event
{
  printf("mouse down\n");
  // Redraw the rectangle.
  [self setNeedsDisplay:YES];
}

- (void)mouseMoved:(NSEvent*)event
{
  printf("mouse moved\n");
  // Redraw the rectangle.
  [self setNeedsDisplay:YES];
}
@end

void
initApp()
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

  [window setTitle:@"My App"];

  // The delegate handles events related to the window.
  WindowDelegate* windowDelegate = [WindowDelegate new];
  [window setDelegate:windowDelegate];

  // The view that draws the contents of the window.
  NSView* view = [[View alloc] initWithFrame:frame];
  [window setContentView:view];

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

  [app activateIgnoringOtherApps:YES];
  [app run];
}
