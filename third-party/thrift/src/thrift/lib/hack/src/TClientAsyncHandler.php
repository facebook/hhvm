<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
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
 *
 */

use namespace FlibSL\{C, Math, Str, Vec}; // @oss-enable

/**
 * This allows clients to perform calls as generators
 */
// @oss-disable: <<Oncalls('thrift')>>
class TClientAsyncHandler {

  /**
   * Called before send in gen_methodName() calls
   * @param string $service_name The caller's Thrift service name
   * @param string $func_name The caller's Thrift method name
   * @param ?IThriftStruct $args The caller's Thrift method's argument
   */
  public async function genBefore(
    string $service_name,
    string $func_name,
    ?IThriftStruct $args = null,
  ): Awaitable<void> {
    // Do nothing
  }

  // Called between the send and recv for gen_methodName() calls
  public async function genWait(
    int $sequence_id,
  )[zoned_local]: Awaitable<void> {
    // Do nothing
  }

  // Called after recv in gen_methodName() calls
  public async function genAfter<<<__Explicit>> TResponse>(
    string $func_name,
    TResponse $response,
  )[zoned_local]: Awaitable<void> {
    // Do nothing
  }

  // Called after recv when gen_methodName() calls throw.
  public async function genOnError(
    string $func_name,
    Exception $ex,
  ): Awaitable<void> {
    // Do nothing
  }

  // Called between the send and recv for streaming gen_methodName() calls.
  // Returns a generator of raw serialized payload bytes (bare structs, no
  // envelope). The handler is responsible for making the first response
  // available on the input transport before this method returns.
  public async function genWaitStream(
    int $sequence_id,
  )[zoned_local]: Awaitable<HH\AsyncGenerator<null, string, void>> {
    return self::genEmptyStream();
  }

  // Called between the send and recv for sink gen_methodName() calls.
  // Returns a sink function that accepts raw serialized payload bytes and
  // returns raw serialized final response bytes (bare structs, no envelope).
  // The handler is responsible for making the first response available
  // on the input transport before this method returns.
  public async function genWaitSink(int $sequence_id)[zoned_local]: Awaitable<
    (function(HH\AsyncGenerator<null, string, void>): Awaitable<string>),
  > {
    return async (
      HH\AsyncGenerator<null, string, void> $_gen,
    )[zoned_local] ==> {
      return '';
    };
  }

  // Called between the send and recv for bidi gen_methodName() calls.
  // Returns a tuple of (stream, sink_func) where:
  //   - stream yields raw serialized payload bytes (server→client direction)
  //   - sink_func accepts a generator of raw serialized payload bytes
  //     (client→server direction) and runs until the generator is exhausted
  // The handler is responsible for making the first response available
  // on the input transport before this method returns.
  public async function genWaitBiDi(int $sequence_id)[zoned_local]: Awaitable<
    (
      HH\AsyncGenerator<null, string, void>,
      (function(
        HH\AsyncGenerator<null, string, void>,
      )[zoned_local]: Awaitable<void>),
    ),
  > {
    return tuple(
      self::genEmptyStream(),
      async (HH\AsyncGenerator<null, string, void> $_gen)[zoned_local] ==> {},
    );
  }

  // Called before stream consumption begins in streaming gen_methodName() calls
  public async function genBeforeStream(string $func_name): Awaitable<void> {
    // Do nothing
  }

  // Called per stream chunk in streaming gen_methodName() calls
  public async function genAfterStream<<<__Explicit>> TStreamResponse>(
    string $func_name,
    TStreamResponse $response,
  )[zoned_local]: Awaitable<void> {
    // Do nothing
  }

  protected static async function genEmptyStream(
  )[zoned_local]: HH\AsyncGenerator<null, string, void> {
    foreach (vec[] as $item) {
      yield $item;
    }
  }
}
