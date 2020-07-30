<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  public function __construct(<<Policied("PRIVATE")>> public int $valuex) {}
}

class Y {
  public function __construct(<<Policied("PUBLIC")>> public int $valuey) {}
}

function f(): void {
  $x = new X(1234);
  if ($x->valuex > 10) {
    $y = new Y(99);
  }
}
