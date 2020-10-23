<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  <<__InferFlows>>
  public function __construct(<<__Policied("PRIVATE")>> public int $valuex) {}

  <<__InferFlows>>
  public static function selfToSelf(): void {
    $x = new self(42);
    $y = new Y(24);

    $y->valuey = $x->valuex;
  }
}

class Y {
  <<__InferFlows>>
  public function __construct(<<__Policied("PUBLIC")>> public int $valuey) {}
}

class Z extends Y {
  <<__InferFlows>>
  public static function parentToSelf(): void {
    $x = new X(42);
    $y = new parent(24);

    $y->valuey = $x->valuex;
  }
}

<<__InferFlows>>
function f(): void {
  $x = new X(1234);
  if ($x->valuex > 10) {
    $y = new Y(99);
  }
}
