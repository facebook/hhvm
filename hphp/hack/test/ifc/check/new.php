<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class X {
  public function __construct(<<Policied("PRIVATE")>> public int $valuex) {}

  public static function selfToSelf(): void {
    $x = new self(42);
    $y = new Y(24);

    $y->valuey = $x->valuex;
  }
}

class Y {
  public function __construct(<<Policied("PUBLIC")>> public int $valuey) {}
}

class Z extends Y {
  public static function parentToSelf(): void {
    $x = new X(42);
    $y = new parent(24);

    $y->valuey = $x->valuex;
  }
}

function f(): void {
  $x = new X(1234);
  if ($x->valuex > 10) {
    $y = new Y(99);
  }
}
