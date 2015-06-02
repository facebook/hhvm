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

class A {}
class B extends A {}

function f((function(B, ...): string) $g): string {
  return $g(new B(), 5);
}

function test(): void {
  f(
    function(A $x, ...): string {
      return "...";
    },
  );
}
