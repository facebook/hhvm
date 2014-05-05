<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function foo(): Awaitable<int> {
  yield 123;
}

/**
 * TODO(glevi): This should error as soon as we remove that hack that says a
 * Continuation is Awaitable
 */
function bar(): Awaitable<int> {
  yield wait_for_result(foo());
}
