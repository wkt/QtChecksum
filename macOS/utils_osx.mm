#include "utils.h"

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

@interface MHFinderServicesProvider : NSObject
{
    OnFinderHandleFiles _handleFiles;
}

- (id)initWithBlock:(OnFinderHandleFiles)handleFiles;
- (void)openForHash:(NSPasteboard *)pboard
           userData:(NSString *)userData error:(NSString **)error;

+ (void)setupServices:(OnFinderHandleFiles)hashFiles;

@end

@implementation MHFinderServicesProvider
- (id)initWithBlock:(OnFinderHandleFiles)handleFiles
{
    self = [super init];
    _handleFiles = handleFiles;
    return  self;
}

- (void)openForHash:(NSPasteboard *)pboard
             userData:(NSString *)userData error:(NSString **)error
{
    if(!_handleFiles)return;

    if(![[pboard types] containsObject:NSFilenamesPboardType]){
        *error = NSLocalizedString(@"Error: bad Pboard type",
                                   @"hash failed");
        return;
    }
    
    NSArray<NSString*> *files = NULL;
    NSArray *items = [pboard pasteboardItems];
    for (NSPasteboardItem *i in items) {
        NSURL *url = [NSURL URLWithString:[i stringForType:@"public.file-url"]];
        if(files == NULL)files = [NSMutableArray arrayWithObject:[url path]];
        else [(NSMutableArray*)files addObject:[url path]];
    }
    QStringList qsFiles;
    for(int i=0;i<files.count;i++){
        qsFiles.append(QString::fromNSString(files[i]));
    }
    _handleFiles(qsFiles);
    
    [pboard clearContents];
}


+(void)setupServices:(OnFinderHandleFiles)hashFiles
{
    [NSApp setServicesProvider:[[MHFinderServicesProvider alloc] initWithBlock:hashFiles]];
    NSUpdateDynamicServices();
}
@end

void setupFinderService(OnFinderHandleFiles func)
{
    [MHFinderServicesProvider setupServices:func];
}
