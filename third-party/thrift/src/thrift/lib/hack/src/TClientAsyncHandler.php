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

/**
 * This allows clients to perform calls as generators
 */
<<Oncalls('thrift')>> // @oss-disable
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
}
