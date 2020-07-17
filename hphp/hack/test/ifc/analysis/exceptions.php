<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  public function __construct(<<Policied("PRIVATE")>> public int $valuex) {}
}

class Y {
  public function __construct(<<Policied("PUBLIC")>> public int $valuey) {}
}

class E extends Exception {}

function f(X $x, Y $y, E $e): void {
  if ($x->valuex > 10) {
    throw $e;
  }
  $y->valuey = 10;
}
