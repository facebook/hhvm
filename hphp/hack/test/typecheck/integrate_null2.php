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

function foo(array<int> $arr): ?array<int> {
  $x = null;
  if (coin_flip()) {
    $x = $arr;
  }
  return $x;
}

function coin_flip(): bool {
  return true;
}
