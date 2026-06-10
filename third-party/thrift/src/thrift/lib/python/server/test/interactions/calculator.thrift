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

/*
 * Test IDL for thrift-python server-side interaction support (single
 * request / single response only). Streaming, sinks, and lifecycle hooks are
 * covered in later commits.
 *
 * Exercises:
 *   - Handshake (factory functions): implicit `performs`, explicit factory,
 *     and a request/response-less factory on a dedicated interaction.
 *   - Request/response inside an interaction: void, scalar, struct, oneway,
 *     and a method that throws.
 *   - An interaction whose server-side factory raises, to exercise
 *     factory-exception propagation.
 *   - A baseline non-interaction method as a regression guard.
 */

package "facebook.com/thrift/python/test/interactions/calculator"

namespace py3 ""
namespace cpp2 cpp2

struct Point {
  1: i32 x;
  2: i32 y;
}

exception NegativeError {
  1: string reason;
}

interaction Counter {
  void add(1: i32 n);
  i32 get();
  void reset();
  // @lint-ignore THRIFTCHECKS avoid-oneway-method (oneway-in-interaction coverage)
  oneway void noop();
  Point getPoint();
  void accumulatePoint(1: Point p);
  // request-response that may throw
  i32 addChecked(1: i32 n) throws (1: NegativeError err);
  // raises an *undeclared* exception (a handler bug); the client should observe
  // an ApplicationError with type UNKNOWN.
  void raiseUnexpected();
  // raises ApplicationError directly from the handler.
  void raiseAppError();
}

// Minimal interaction to test the request/response-less factory path in
// isolation from `Counter`.
interaction Heartbeat {
  void ping();
}

// Interaction whose server-side factory (`createBoom`) intentionally raises, so
// the test can assert the factory exception propagates to the client instead of
// being swallowed.
interaction Boom {
  void noop();
}

service Calculator {
  // baseline non-interaction method (regression guard)
  i32 echo(1: i32 n);

  // implicit factory: client gets `createCounter()`
  performs Counter;

  // implicit factory whose server-side `createBoom` raises
  performs Boom;

  // explicit factory function (no leading response)
  Counter newCounter();

  // explicit factory for the request/response-less `Heartbeat` interaction
  Heartbeat newHeartbeat();

  // explicit factory whose server-side `createBoom` raises (exercises the
  // maybeFulfillTilePromise factory-exception path)
  Boom newBoom();
}
