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
  while ($x) {
    foo($x);
  }
}

function test2(?A $x): void {
  while (!$x) {
  }
  foo($x);
}

function test3(?A $x): void {
  for (; $x; ) {
    foo($x);
  }
}

function test4(?A $x): void {
  for (; !$x; ) {
  }
  foo($x);
}

function test5(?A $x): void {
  if ($x) {
    foo($x);
  }
}

function test6(?A $x): void {
  if (!$x) {
  } else {
    foo($x);
  }
}
