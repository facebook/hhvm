 <?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
/**
 *  Copyright 2012-2013 Facebook.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.g
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

async function funtimes() : Awaitable<int> {
  return 314159;
}

async function animals(): Awaitable<string> {
  return "llama llama duck";
}

class PositionOrVelocity {}

async function heisenberg(): Awaitable<PositionOrVelocity> {
  return new PositionOrVelocity();
}

function test() : Awaitable<(int, string, PositionOrVelocity)> {
  return genva(funtimes(), animals(), heisenberg());
}
