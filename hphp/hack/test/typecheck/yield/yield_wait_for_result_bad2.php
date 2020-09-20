<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function foo(): Awaitable<int> {
  yield 123;
}

/**
 * TODO(glevi): This should error as soon as we remove that hack that says a
 * Generator is Awaitable
 */
function bar(): Awaitable<int> {
  yield wait_for_result(foo());
}
