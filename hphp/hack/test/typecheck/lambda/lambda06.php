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

function foo(Map<string, int> $map): void {
  $func = $x ==> $x + 1;
  $routine = $y ==> {
    $ret = Map {};
    foreach ($y as $k => $v) {
      $ret[$k] = $func($v); // $func is captured
    }
  };
  $routine($map);
}
