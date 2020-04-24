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

function f((function(int, float...): string) $g): string {
  return $g(5, 23.3);
}

<<__EntryPoint>>
function test(): void {
  // Should not accept this!
  f(
    function(int $x, string $y, float ...$args): string {
      return $x."<-";
    },
  );
}
