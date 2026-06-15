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
 * Test IDL for thrift-python server-side interaction support (request/response
 * and server streaming). Sinks, bidi, and lifecycle hooks are covered in later
 * commits.
 *
 * Exercises:
 *   - Handshake (factory functions): implicit `performs`, explicit factory,
 *     and a request/response-less factory on a dedicated interaction.
 *   - Request/response inside an interaction: void, scalar, struct, oneway,
 *     and a method that throws.
 *   - Server streaming inside an interaction: with and without an initial
 *     response, depending on per-session state, and a stream that raises a
 *     declared exception mid-stream.
 *   - An interaction whose server-side factory raises, to exercise
 *     factory-exception propagation.
 *   - Baseline non-interaction methods (request/response and stream) as
 *     regression guards.
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

// Initial response returned alongside a freshly-created `Counter` interaction by
// the `initializedCounter` factory. Carries the starting value so the test can
// assert the factory's initial response and the per-session Tile state both
// derive from the same factory argument.
struct CounterSnapshot {
  1: i32 value;
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
  // server stream from an interaction: initial response is the current value,
  // stream yields `count` successive values starting at the current value. Both
  // depend on per-session state, so this exercises per-session dispatch.
  i32, stream<i32> ticks(1: i32 count);
  // server stream with no initial response; yields the current value's worth of
  // ticks (0..value-1), also dependent on per-session state.
  stream<i32> drain();
  // server stream that yields `count` values then raises a declared exception
  // mid-stream, exercising the stream's UserExceptionMeta path.
  i32, stream<i32 throws (1: NegativeError err)> ticksThenFail(1: i32 count);
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

// Interaction whose only method is a sink. Sink/bidi interaction methods are
// gated out of the generated interface (deferred to a later change), so *all* of
// this interaction's methods are gated and the generated `SinkOnlyInterface`
// body is empty -- exercising the `has_interface_methods? == false` codegen path
// that must emit a bare `pass` (and a still-valid, subclassable interface).
interaction SinkOnly {
  // @lint-ignore THRIFTCHECKS avoid-sink-method (empty-interface codegen coverage)
  sink<i32, i32> collect();
}

service Calculator {
  // baseline non-interaction methods (regression guard)
  i32 echo(1: i32 n);
  i32, stream<i32> serviceTicks(1: i32 count);

  // implicit factory: client gets `createCounter()`
  performs Counter;

  // implicit factory whose server-side `createBoom` raises
  performs Boom;

  // explicit factory function (no leading response)
  Counter newCounter();

  // explicit factory function *with* a leading initial response: returns both
  // the new `Counter` interaction and a `CounterSnapshot`. The handler builds
  // the Tile from the factory argument (so per-session state derives from
  // `start`) and returns `tuple[Counter, CounterSnapshot]`, exercising the
  // interaction-factory-with-initial-response (TileAndResponse) path. The
  // declared `throws` lets the test exercise error propagation from the factory
  // itself: a declared exception surfaces as-is, an undeclared one as an
  // ApplicationError.
  Counter, CounterSnapshot initializedCounter(1: i32 start) throws (
    1: NegativeError err,
  );

  // Stream interaction factory (`Interaction, Response, stream<T>`). Regression
  // guard for the tile-unpack-before-await codegen bug: the initial-response
  // Tile install is gated to the request/response path, so a stream factory's
  // Tile comes from the zero-arg `createCounter` (state starts at the default).
  Counter, i32, stream<i32> streamingCounter(1: i32 count);

  // explicit factory for the request/response-less `Heartbeat` interaction
  Heartbeat newHeartbeat();

  // explicit factory whose server-side `createBoom` raises (exercises the
  // maybeFulfillTilePromise factory-exception path)
  Boom newBoom();

  // factory for the sink-only interaction; only here so `createSinkOnly` and the
  // empty `SinkOnlyInterface` are generated and exercised at build/import time.
  performs SinkOnly;
}
