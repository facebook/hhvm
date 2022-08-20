/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace cpp2 testutil.testservice

exception TestServiceException {
  1: string message;
}

service IntermHeaderService {
  i32 callAdd(1: i32 x); // forwards to TestService::add()
}

service TestService {
  i32 sumTwoNumbers(1: i32 x, 2: i32 y);

  i32 add(1: i32 x);

  oneway void addAfterDelay(1: i32 delayMs, 2: i32 x);

  oneway void onewayThrowsUnexpectedException(1: i32 delayMs);

  void throwExpectedException(1: i32 x) throws (1: TestServiceException e);

  void throwUnexpectedException(1: i32 x) throws (1: TestServiceException e);

  void sleep(1: i32 timeMs);

  void headers();

  string hello(1: string name);

  void checkPort(1: i32 port);

  string echo(1: binary (cpp2.type = "folly::IOBuf") val);

  oneway void onewayLogBlob(1: binary (cpp2.type = "folly::IOBuf") val);
}
