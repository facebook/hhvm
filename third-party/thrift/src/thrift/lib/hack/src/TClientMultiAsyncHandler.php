<?hh
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

// @oss-disable: <<Oncalls('thrift')>>
final class TClientMultiAsyncHandler extends TClientAsyncHandler {
  private dict<string, TClientAsyncHandler> $handlers = dict[];

  public function __construct()[] {
  }

  public function addHandler(
    string $key,
    TClientAsyncHandler $handler,
  )[write_props]: this {
    $this->handlers[$key] = $handler;
    return $this;
  }

  public function getHandler(string $key)[]: TClientAsyncHandler {
    return $this->handlers[$key];
  }

  public function removeHandler(string $key)[write_props]: TClientAsyncHandler {
    $handler = $this->getHandler($key);
    unset($this->handlers[$key]);
    return $handler;
  }

  public function getHandlers()[]: readonly dict<string, TClientAsyncHandler> {
    return readonly $this->handlers;
  }

  <<__Override>>
  public async function genBefore(
    string $service_name,
    string $func_name,
    ?IThriftStruct $args = null,
  ): Awaitable<void> {
    await Vec\map_async(
      $this->handlers,
      async $handler ==>
        await $handler->genBefore($service_name, $func_name, $args),
    );
  }

  <<__Override>>
  public async function genWait(
    int $sequence_id,
  )[zoned_shallow]: Awaitable<void> {
    await Vec\map_async(
      $this->handlers,
      async $handler ==> await $handler->genWait($sequence_id),
    );
  }

  <<__Override>>
  public async function genAfter<<<__Explicit>> TResponse>(
    string $func_name,
    TResponse $response,
  )[zoned_local]: Awaitable<void> {
    await Vec\map_async(
      $this->handlers,
      async ($handler)[zoned_local] ==>
        await $handler->genAfter<TResponse>($func_name, $response),
    );
  }

  <<__Override>>
  public async function genWaitStream(
    int $sequence_id,
  )[zoned_local]: Awaitable<HH\AsyncGenerator<null, string, void>> {
    $results = await Vec\map_async(
      $this->handlers,
      async ($handler)[zoned_local] ==>
        await $handler->genWaitStream($sequence_id),
    );
    return (
      async function()[zoned_local] use ($results) {
        foreach ($results as $result_gen) {
          foreach ($result_gen await as $payload) {
            yield $payload;
          }
        }
      }
    )();
  }

  // Note: This assumes at most one handler consumes the generator.
  // Async generators can only be consumed once. Most handlers (logging,
  // metrics, tracing) should not override genWaitSink() or should return
  // a no-op function. Only the actual transport handler
  // should consume the generator.
  <<__Override>>
  public async function genWaitSink(int $sequence_id)[zoned_local]: Awaitable<
    (function(HH\AsyncGenerator<null, string, void>): Awaitable<string>),
  > {
    $results = await Vec\fb\map_and_filter_nulls_async(
      $this->handlers,
      async ($handler)[zoned_local] ==>
        await $handler->genWaitSink($sequence_id),
    );
    return async (HH\AsyncGenerator<null, string, void> $gen)[zoned_local] ==> {
      $final_response = await Vec\map_async(
        $results,
        async ($result)[zoned_local] ==> await $result($gen),
      );
      foreach ($final_response as $res) {
        if (!Str\is_empty($res)) {
          return $res;
        }
      }
      return '';
    };
  }
}
