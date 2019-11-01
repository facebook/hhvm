<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function __construct(public int $x) {}
}

function foo(?A $a): void {
  $f = $x ==> expect<?string>($x?->x);
  $f($a);
}

function expect<T>(T $_): void {}
