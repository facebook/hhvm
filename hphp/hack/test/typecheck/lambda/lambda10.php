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

function str(string $k): void {}
function inty(int $k): void {}

function foo(): void {
  $x = 2;
  $z = 3;
  $y = () ==> {
    $z = "asd";
    return $z;
  };
  str($y());
  inty($z);
}
