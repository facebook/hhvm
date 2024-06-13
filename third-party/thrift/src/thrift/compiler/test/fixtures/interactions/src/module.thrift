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

namespace java test.fixtures.interactions
namespace java.swift test.fixtures.interactions
namespace py test.fixtures.interactions
namespace py3 test.fixtures.interactions

exception CustomException {
  1: string message;
}

interaction MyInteraction {
  i32 frobnicate() throws (1: CustomException ex);
  oneway void ping();
  stream<bool> truthify();
  set<i32>, sink<string, binary> encode();
}

interaction MyInteractionFast {
  i32 frobnicate();
  oneway void ping();
  stream<bool> truthify();
  set<i32>, sink<string, binary> encode();
} (process_in_event_base)

interaction SerialInteraction {
  void frobnicate();
} (serial)

service MyService {
  performs MyInteraction;
  performs MyInteractionFast;
  performs SerialInteraction;
  void foo();

  MyInteraction interact(1: i32 arg);
  MyInteractionFast, i32 interactFast();
  SerialInteraction, i32, stream<i32> serialize();
}

service Factories {
  void foo();

  MyInteraction interact(1: i32 arg);
  MyInteractionFast, i32 interactFast();
  SerialInteraction, i32, stream<i32> serialize();
}

service Perform {
  performs MyInteraction;
  performs MyInteractionFast;
  performs SerialInteraction;
  void foo();
}
