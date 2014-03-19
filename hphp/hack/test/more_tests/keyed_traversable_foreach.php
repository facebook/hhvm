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

function foo1(KeyedTraversable $p) {
  foreach ($p as $v) {
    f2($v);
  }
}

function foo2(KeyedTraversable $p) {
  foreach ($p as $k => $v) {
    f1($k);
    f2($v);
  }
}

function foo3(KeyedTraversable<int, string> $p) {
  foreach ($p as $v) {
    f2($v);
  }
}

function foo4(KeyedTraversable<int, string> $p) {
  foreach ($p as $k => $v) {
    f2($v);
  }
}

function arr1(array $arr) {
// foo1($arr);
  foo3($arr);
}
function arr2(array<string> $arr) {
//  foo1($arr);
  foo3($arr);
}
function arr3(array<int, string> $arr) {
//  foo1($arr);
  foo3($arr);
}
function map1(Map $map) {
  foo1($map);
  foo3($map);
}
function map2(Map<int, string> $map) {
  foo1($map);
  foo3($map);
}
function vec1(Vector $v) {
  foo1($v);
  foo3($v);
}
function vec2(Vector<string> $v) {
  foo1($v);
  foo3($v);
}

function f1(int $k) {}

function f2(string $v) {}
