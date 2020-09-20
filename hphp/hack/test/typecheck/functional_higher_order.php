<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function f<T>((function(T): T) $g, T $x): T {
  return ($g)($x);
}

function test(): void {
  f(
    function($x) {
      return $x + 1.0;
    },
    0,
  );
}
