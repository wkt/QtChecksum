#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#ifndef ___MAIN___
#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@end
#endif

#ifdef __cplusplus
extern "C" {
#endif

int AppDelegate_init(int argc, const char *argv[]);

#ifdef __cplusplus
}
#endif

                         
#endif // APPDELEGATE_H
