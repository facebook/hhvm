<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function foo1(array $arr) {
  foreach ($arr as $v) {
    f2($v);
  }
  $arr[] = 'meh';
  $arr[10] = 'meh';
  f2($arr[10]);
  foo1($arr);
  foo3($arr);
  foo5($arr);
}

function foo2(array $arr) {
  foreach ($arr as $k => $v) {
    f1($k);
    f2($v);
  }
}

function foo3(array<string> $arr) {
  foreach ($arr as $v) {
    f2($v);
  }
  $arr[] = 'meh';
  $arr[10] = 'meh';
  f2($arr[10]);
  foo1($arr);
  foo3($arr);
  foo5($arr);
}

function foo4(array<string> $arr) {
  foreach ($arr as $k => $v) {
    f1($k);
    f2($v);
  }
}

function foo5(array<int, string> $arr) {
  foreach ($arr as $v) {
    f2($v);
  }
  $arr[10] = 'meh';
  f2($arr[10]);
  foo1($arr);
  foo5($arr);
}

function foo6(array<int, string> $arr) {
  foreach ($arr as $k => $v) {
    f1($k);
    f2($v);
  }
}

function f1(int $k) {}

function f2(string $v) {}
