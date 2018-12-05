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

function blah(int $k): void {}

function foo(): void {
  // transitive lambda captures
  $x = Vector { 1, 2, 3 };
  $a = () ==> {
    return () ==> {
      return $idx ==> $x[$idx];
    };
  };
  $func = $a();
  $func = $func();
  $hey = $func(1);
  blah($hey);
}
