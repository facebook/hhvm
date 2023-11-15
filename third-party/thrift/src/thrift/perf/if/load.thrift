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
namespace go apache.thrift.test.load
namespace py apache.thrift.test.load
namespace py3 thrift.perf
namespace py.asyncio apache.thrift.test.asyncio.load
namespace java.swift org.apache.swift.thrift.perf

include "thrift/annotation/cpp.thrift"

exception LoadError {
  1: i32 code;
}

struct BigStruct {
  1: string stringField;
  2: list<string> stringList;
}

service LoadTest {
  // Methods for testing server performance
  // Fairly simple requests, to minimize serialization overhead

  /**
   * noop() returns immediately, to test behavior of fast, cheap operations
   */
  @cpp.ProcessInEbThreadUnsafe
  void noop();
  @cpp.ProcessInEbThreadUnsafe
  oneway void onewayNoop();

  /**
   * asyncNoop() is like noop() except for one minor difference in the async
   * implementation.
   *
   * In the async handler, noop() invokes the callback immediately, while
   * asyncNoop() uses runInLoop() to invoke the callback.
   */
  @cpp.ProcessInEbThreadUnsafe
  void asyncNoop();

  /**
   * sleep() waits for the specified time period before returning,
   * to test slow, but not CPU-intensive operations
   */
  @cpp.ProcessInEbThreadUnsafe
  void sleep(1: i64 microseconds);
  @cpp.ProcessInEbThreadUnsafe
  oneway void onewaySleep(1: i64 microseconds);

  /**
   * burn() uses as much CPU as possible for the desired time period,
   * to test CPU-intensive operations
   */
  void burn(1: i64 microseconds);
  oneway void onewayBurn(1: i64 microseconds);

  /**
   * badSleep() is like sleep(), except that it exhibits bad behavior in the
   * async implementation.
   *
   * Instead of returning control to the event loop, it actually sleeps in the
   * worker thread for the specified duration.  This tests how well the thrift
   * infrastructure responds to badly written handler code.
   */
  @cpp.ProcessInEbThreadUnsafe
  void badSleep(1: i64 microseconds);

  /**
   * badBurn() is like burn(), except that it exhibits bad behavior in the
   * async implementation.
   *
   * The normal burn() call periodically yields to the event loop, while
   * badBurn() does not.  This tests how well the thrift infrastructure
   * responds to badly written handler code.
   */
  @cpp.ProcessInEbThreadUnsafe
  void badBurn(1: i64 microseconds);

  /**
   * throw an error
   */
  @cpp.ProcessInEbThreadUnsafe
  void throwError(1: i32 code) throws (1: LoadError error);

  /**
   * throw an unexpected error (not declared in the .thrift file)
   */
  @cpp.ProcessInEbThreadUnsafe
  void throwUnexpected(1: i32 code);

  /**
   * throw an error in a oneway call,
   * just to make sure the internal thrift code handles it properly
   */
  @cpp.ProcessInEbThreadUnsafe
  oneway void onewayThrow(1: i32 code);

  /**
   * Send some data to the server.
   *
   * The data is ignored.  This is primarily to test the server I/O
   * performance.
   */
  @cpp.ProcessInEbThreadUnsafe
  void send(1: binary data);

  /**
   * Send some data to the server.
   *
   * The data is ignored.  This is primarily to test the server I/O
   * performance.
   */
  @cpp.ProcessInEbThreadUnsafe
  oneway void onewaySend(1: binary data);

  /**
   * Receive some data from the server.
   *
   * The contents of the data are undefined.  This is primarily to test the
   * server I/O performance.
   */
  @cpp.ProcessInEbThreadUnsafe
  binary recv(1: i64 bytes);

  /**
   * Send and receive data
   */
  @cpp.ProcessInEbThreadUnsafe
  binary sendrecv(1: binary data, 2: i64 recvBytes);

  /**
   * Echo data back to the client.
   */
  binary echo(1: binary data);

  /**
   * Add the two integers
   */
  @cpp.ProcessInEbThreadUnsafe
  i64 add(1: i64 a, 2: i64 b);

  /**
   * Send a large container of large structs
   */
  @cpp.ProcessInEbThreadUnsafe
  void largeContainer(1: list<BigStruct> items);

  /**
   * Send a large container, iterate all fields on all structs, echo back
   */
  @cpp.ProcessInEbThreadUnsafe
  list<BigStruct> iterAllFields(1: list<BigStruct> items);
}
