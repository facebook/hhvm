<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function __construct(public int $x) {}
}

function test(?A $a): void {
  $x = Vector {};
  while (true) {
    if (!$x->isEmpty()) {
      expect<?int>($x[0]?->x);
      break;
    } else {
      $x[] = $a;
    }
  }
}

function expect<T>(T $_): void {}
