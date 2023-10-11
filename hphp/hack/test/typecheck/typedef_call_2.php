<?hh
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

newtype A<T as arraykey> = (function(dict<T, shape('a' => T, 'b' => float)>): T);

function test(A<int> $x): int {
  return $x(dict[1 => shape('a' => 2, 'b' => 3.0)]);
}
