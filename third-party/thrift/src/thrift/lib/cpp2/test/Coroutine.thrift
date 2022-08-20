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

namespace cpp2 apache.thrift.test

struct SumRequest {
  1: i32 x;
  2: i32 y;
}

struct SumResponse {
  1: i32 sum;
}

exception Ex {}

service Coroutine {
  SumResponse computeSumNoCoro(1: SumRequest request);

  SumResponse computeSum(1: SumRequest request) (cpp.coroutine);
  i32 computeSumPrimitive(1: i32 x, 2: i32 y) (cpp.coroutine);

  void computeSumVoid(1: i32 x, 2: i32 y) (cpp.coroutine);

  SumResponse computeSumUnimplemented(1: SumRequest request) (cpp.coroutine);
  i32 computeSumUnimplementedPrimitive(1: i32 x, 2: i32 y) (cpp.coroutine);

  SumResponse computeSumThrows(1: SumRequest request) (cpp.coroutine);
  i32 computeSumThrowsPrimitive(1: i32 x, 2: i32 y) (cpp.coroutine);

  i32 noParameters() (cpp.coroutine);

  SumResponse implementedWithFutures() (cpp.coroutine);
  i32 implementedWithFuturesPrimitive() (cpp.coroutine);

  i32 takesRequestParams() (cpp.coroutine);

  oneway void onewayRequest(1: i32 x) (cpp.coroutine);

  SumResponse computeSumThrowsUserEx(1: SumRequest request) throws (1: Ex e) (
    cpp.coroutine,
  );
  i32 computeSumThrowsUserExPrimitive(1: i32 x, 2: i32 y) throws (1: Ex e) (
    cpp.coroutine,
  );
}
