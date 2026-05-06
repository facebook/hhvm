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

class A {}

function foo(A $x): void {}

function test1(?A $x): void {
  while ($x !== null) {
    foo($x);
  }
}

function test2(?A $x): void {
  while ($x === null) {
  }
  foo($x);
}

function test3(?A $x): void {
  for (; $x !== null; ) {
    foo($x);
  }
}

function test4(?A $x): void {
  for (; $x === null; ) {
  }
  foo($x);
}

function test5(?A $x): void {
  if ($x !== null) {
    foo($x);
  }
}

function test6(?A $x): void {
  if ($x === null) {
  } else {
    foo($x);
  }
}
