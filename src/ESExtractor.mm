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
  return [NSDictionary dictionaryWithContentsOfURL:[self tempFile]];
}

- (NSURL *)tempFile {
  return ({
    NSURL *tempDirectory = [NSURL fileURLWithPath:NSTemporaryDirectory()];
    [tempDirectory URLByAppendingPathComponent:@"Output.json"];
  });
}

@end
