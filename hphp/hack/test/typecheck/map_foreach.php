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

// TODO: Fix A<T> as A
//
//function foo1(Map $map) {
//  foreach ($map as $v) {
//    f2($v);
//  }
//  $map[] = 'meh';
//  $map[10] = 'meh';
//  f2($map[10]);
//}

//function foo2(Map $map) {
//  foreach ($map as $k => $v) {
//    f1($k);
//    f2($v);
//  }
//}

function foo3(Map<int, string> $map) {
  foreach ($map as $v) {
    f2($v);
  }
//  $map[] = 'meh';
//  $map[10] = 'meh';
  f2($map[10]);
}

function foo4(Map<int, string> $map) {
  foreach ($map as $k => $v) {
    f1($k);
    f2($v);
  }
}

function f1(int $k) {}

function f2(string $v) {}
