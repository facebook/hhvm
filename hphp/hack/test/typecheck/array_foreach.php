<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function foo3(varray<string> $arr) {
  foreach ($arr as $v) {
    f2($v);
  }
  $arr[] = 'meh';
  $arr[10] = 'meh';
  f2($arr[10]);
  foo3($arr);
  foo5($arr);
}

function foo4(varray<string> $arr) {
  foreach ($arr as $k => $v) {
    f1($k);
    f2($v);
  }
}

function foo5(darray<int, string> $arr) {
  foreach ($arr as $v) {
    f2($v);
  }
  $arr[10] = 'meh';
  f2($arr[10]);
  foo5($arr);
}

function foo6(darray<int, string> $arr) {
  foreach ($arr as $k => $v) {
    f1($k);
    f2($v);
  }
}

function f1(int $k) {}

function f2(string $v) {}
