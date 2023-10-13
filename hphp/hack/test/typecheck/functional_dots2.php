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

function f((function(int, string, mixed...): string) $g): string {
  return $g(5, 'five');
}

function test(): void {
  f(
    function(int $x, mixed ...$args): string {
      return $x."<-";
    },
  );
}
