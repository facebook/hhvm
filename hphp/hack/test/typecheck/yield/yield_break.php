<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function test(bool $b): Generator<int, int, void> {
  yield 0;
  if ($b) {
    yield break;
  }
  yield 1;
}
