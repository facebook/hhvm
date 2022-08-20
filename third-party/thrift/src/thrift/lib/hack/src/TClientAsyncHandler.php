<?hh
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
 *
 * @package thrift
 */

/**
 * This allows clients to perform calls as generators
 */
class TClientAsyncHandler {
  // Called before send in gen_methodName() calls
  public async function genBefore(): Awaitable<void> {
    // Do nothing
  }

  // Called between the send and recv for gen_methodName() calls
  public async function genWait(int $sequence_id): Awaitable<void> {
    // Do nothing
  }

  // Called after recv in gen_methodName() calls
  public async function genAfter(): Awaitable<void> {
    // Do nothing
  }
}
