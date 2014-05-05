<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

function gen(Continuation<int> $x): Continuation<int> {
  $i = 0;
  foreach($x as $k => $v) {
    yield $v;
  }
  $arr = Vector {1, 2, 3};
  foreach($arr as $k => $v) {
    yield $v;
  }
  $map = Map {1 => 1, 2 => 2, 3 => 3};
  foreach($arr as $k => $v) {
    yield $v;
  }
}

function dump(): Continuation<int> {
  yield 33;
  yield 41;
  yield 42;
  yield 43;
}


function bad(): Continuation<int> {
  if(false) {
    yield 1;
  }
}

function main(): void {
  $x = gen(dump());
  $acc = 0;
  foreach($x as $k => $v) {
    $acc += $k + $v;
  }
  if($acc === 216) {
    echo 'OK';
  }
  else {
    echo 'Failure: test_yield.1';
  }
  $y = bad();
  foreach($y as $v) {
    echo 'Failure: test_yield.2';
  }
  $y = bad();
  foreach($y as $v) {
    echo 'Failure: test_yield.2';
  }

}
