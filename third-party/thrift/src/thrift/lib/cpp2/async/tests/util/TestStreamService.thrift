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
exception UserDefinedException {}

service TestStreamService {
  stream<i32> range(1: i32 from, 2: i32 to);
  stream<i32> rangeThrow(1: i32 from, 2: i32 to);
  stream<i32 throws (1: UserDefinedException e)> rangeThrowUDE(
    1: i32 from,
    2: i32 to,
  );
  stream<i32> rangeWaitForCancellation(1: i32 from, 2: i32 to);
  stream<i32> uncompletedPublisherDestructor();
  stream<i32> uncompletedPublisherMoveAssignment();
}
