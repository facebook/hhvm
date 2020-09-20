<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T as shape('x' => ?int, ...)> {
  public function test(T $s): int {
    return Shapes::idx($s, 'x', 0);
  }
}
