#include "appdelegate.h"

@interface AppDelegate ()

@end

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    NSLog(@"%s",__PRETTY_FUNCTION__);
}
    
@end


int AppDelegate_init(int argc, const char *argv[])
{
    NSApplication *app = [NSApplication sharedApplication];
    app.delegate = [[AppDelegate alloc] init];
    NSLog(@"%s",__PRETTY_FUNCTION__);
    return NSApplicationMain(argc, argv);
}
