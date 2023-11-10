/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace cpp2 apache.thrift.test

include "thrift/annotation/cpp.thrift"

include "thrift/annotation/thrift.thrift"

struct Point {
  1: i32 x;
  2: i32 y;
}

interaction Addition {
  void accumulatePrimitive(1: i32 a);
  void accumulatePoint(1: Point a);
  i32 getPrimitive();
  Point getPoint();
  oneway void noop();
}

@cpp.ProcessInEbThreadUnsafe
interaction AdditionFast {
  void accumulatePrimitive(1: i32 a);
  void accumulatePoint(1: Point a);
  i32 getPrimitive();
  Point getPoint();
  oneway void noop();
}

@thrift.Serial
interaction SerialAddition {
  void accumulatePrimitive(1: i32 a);
  i32 getPrimitive();
  stream<i32> waitForCancel();
}

service Calculator {
  performs Addition;
  performs AdditionFast;
  performs SerialAddition;
  i32 addPrimitive(1: i32 a, 2: i32 b);

  Addition newAddition();
  Addition, i32 initializedAddition(1: i32 a);
  Addition, string stringifiedAddition(1: i32 a);
  AdditionFast fastAddition();
  @cpp.ProcessInEbThreadUnsafe
  AdditionFast veryFastAddition();
}

interaction Streaming {
  stream<i32> generatorStream();
  stream<i32> publisherStream();
  sink<i32, byte> _sink();
}

service Streamer {
  performs Streaming;
}
