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

function test_vector(): void {
  $x = Vector {1, 2, 3};
  $acc = 0;
  foreach($x as $v) {
    $acc = $acc + $v;
  }
  if($acc === 6) {
    echo 'OK';
  }
  else {
    echo 'FAILURE: test3.1';
  }
}

function test_map(): void {
  $x = Map {1 => 1, 2 => 2, 3 => 3};
  $acc = 0;
  foreach($x as $k => $v) {
    $acc += $k + $v;
  }
  if($acc === 12) {
    echo 'OK';
  }
  else {
    echo 'FAILURE: test3.2';
  }
}

function test_append(): void {
  $x = Vector {};
  $x[] = 1;
  $x[] = 2;
  $x[] = 3;
  $acc = 0;
  foreach($x as $k => $v) {
    $acc += $k + $v;
  }
  if($acc === 9) {
    echo 'OK';
  }
  else {
    echo 'FAILURE: test3.3';
  }
}

function test_tuple(): void {
  $x = tuple(tuple(1, 3), 2);
  list(list($y, $u), $z) = $x;
  if ($y === 1 && $u === 3 && $z === 2) {
    echo 'OK';
  }
  else {
    echo 'FAILURE: test3.4';
  }
}

function main(): void {
  test_vector();
  test_map();
  test_append();
  test_tuple();
}
