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

class A {}
class B extends A {}

function f((function(B, mixed...): string) $g): string {
  return $g(new B(), 5);
}

function test(): void {
  f(
    function(A $x, mixed ...$args): string {
      return "...";
    },
  );
}
