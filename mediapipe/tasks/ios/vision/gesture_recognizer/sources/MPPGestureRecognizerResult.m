// Copyright 2023 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#import "mediapipe/tasks/ios/vision/gesture_recognizer/sources/MPPGestureRecognizerResult.h"

@implementation MPPGestureRecognizerResult

- (instancetype)initWithLandmarks:(NSArray<NSArray<MPPLandmark *> *> *)landmarks
                   worldLandmarks:(NSArray<NSArray<MPPLandmark *> *> *)worldLandmarks
                       handedness:(NSArray<NSArray<MPPCategory *> *> *)handedness
                         gestures:(NSArray<NSArray<MPPCategory *> *> *)gestures
          timestampInMilliseconds:(NSInteger)timestampInMilliseconds {
  self = [super initWithTimestampMs:timestampInMilliseconds];
  if (self) {
    _landmarks = landmarks;
    _worldLandmarks = worldLandmarks;
    _handedness = handedness;
    _gestures = gestures;
  }
  return self;
}

@end
