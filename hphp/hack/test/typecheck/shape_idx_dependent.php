<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  abstract const type T as shape('x' => ?int, ...);

  public function test(this::T $s): int {
    return Shapes::idx($s, 'x', 0);
  }
}
