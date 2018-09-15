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

type f<Ty> = (function(Ty): Ty);

function mapp<Ty>(Vector<Ty> $arr, f<Ty> $fun): void {}

function foo(): void {
  $func = $y ==> $y + 1;
  $func = $y ==> $y + 2;
  $func = $y ==> {
    return $y + 2;
  };
  $func = $y ==> {
    return $y - 2;
  };
  $x = Vector { 1, 2, 3 };
  mapp($x, $func);
}
