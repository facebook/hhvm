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

function f() /* : TAny */ { return 42; }

function getArr(): varray<int> { return vec[]; }

function g(): void {
  $a = getArr();
  $b = vec[];
  $idx = f();

  // not entirely sure why, but this if statement is needed to trigger an error
  // in a naive implementation of the trivial strict equality check
  if (true) {
  }

  /* This is not a trivial strict equality check [4118]: $idx will be inferred
   * to be an int inside the if block, but right now it is still TAny. This
   * test verifies that we are doing the trivial strict equality test at the
   * right point */
  if ($idx !== false) {
    $b[] = $a[$idx];
  }
}

function h(?int $a, ?string $b): bool {
  // Not a trivial comparison since both $a and $b can be null
  return $a === $b;
}
