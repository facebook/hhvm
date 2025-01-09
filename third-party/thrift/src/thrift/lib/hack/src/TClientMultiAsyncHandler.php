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

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

<<Oncalls('thrift')>> // @oss-disable
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
}
