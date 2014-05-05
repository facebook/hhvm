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
//function foo1(Vector $vec) {
//  foreach ($vec as $v) {
//    f2($v);
//  }
//  $vec[] = 'meh';
//  $vec[10] = 'meh';
//  f2($vec[10]);
//}

//function foo2(Vector $vec) {
//  foreach ($vec as $k => $v) {
//    f1($k);
//    f2($v);
//  }
//}

function foo3(Vector<string> $vec) {
  foreach ($vec as $v) {
    f2($v);
  }
  $vec[] = 'meh';
  $vec[10] = 'meh';
  f2($vec[10]);
}

function foo4(Vector<string> $vec) {
  foreach ($vec as $k => $v) {
    f1($k);
    f2($v);
  }
}

function f1(int $k) {}

function f2(string $v) {}
