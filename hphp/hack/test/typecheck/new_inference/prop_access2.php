<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

function test(A $a): void {
  $x = Vector {};
  while (true) {
    if (!$x->isEmpty()) {
      $x[0]->x;
      break;
    } else {
      $x[] = $a;
    }
  }
}

function expect<T>(T $_): void {}
