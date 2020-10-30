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
function g(X $x, Y $y): void {
  if ($x->valuex > 10) {
    throw new Exception("test", 5, null); // TODO: once we support default args, don't need the extra args
  }
  $y->valuey = 10;
}
