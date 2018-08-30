//
//  ESExtractor.m
//  ESMusicExtractor
//
//  Created by Ragnar Hrafnkelsson on 07/08/2018.
//  Copyright © 2018 Reactify. All rights reserved.
//

#import "streaming_extractor_music.h"
#import "ESExtractor.h"

@implementation ESExtractor

- (NSDictionary *)analyseTrack:(NSURL *)url {
  essentia_main(url.path.UTF8String, [self tempFile].path.UTF8String, "");
  NSData* data = [NSData dataWithContentsOfURL:[self tempFile]];
  @try {
    return [NSJSONSerialization JSONObjectWithData:data options:kNilOptions error:nil];
  } @catch (NSException *exception) {
    return nil;
  }
}

- (NSURL *)tempFile {
  return [[self tempDirectory] URLByAppendingPathComponent:@"Output.json"];
}

- (NSURL *)tempDirectory {
  return [NSURL fileURLWithPath:NSTemporaryDirectory()];
}

@end
