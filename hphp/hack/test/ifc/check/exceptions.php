<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  <<__InferFlows>>
  public function __construct(<<__Policied("PRIVATE")>> public int $valuex) {}
}

class Y {
  <<__InferFlows>>
  public function __construct(<<__Policied("PUBLIC")>> public int $valuey) {}
}

<<__InferFlows>>
function f(X $x, Y $y, Exception $e): void {
  if ($x->valuex > 10) {
    throw $e;
  }
  $y->valuey = 10;
}
