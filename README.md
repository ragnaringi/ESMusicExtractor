# ESMusicExtractor
Essentia Music Extractor on iOS

Build the Xcode project or run Carthage `github "ragnaringi/ESMusicExtractor"` and add ESMusicExtractor.framework into your project.

Provide and audio file to analyse and parse the resulting dictionary for features:
```
#import <ESMusicExtractor/ESExtractor.h>

NSURL *inputURL = [[NSBundle mainBundle].resourceURL URLByAppendingPathComponent:@"Test.m4a"];
  
ESExtractor* extractor = [ESExtractor new];
NSDictionary* info = [extractor analyseTrack:inputURL];
NSLog(@"%@", info[@"rhythm"][@"beats_position"]);
```
