<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const type T = int;

  public static function f(): void {
    $a = 4;
    if ($a is this::T) {
      $a as this::T;
    }

    $b = shape('a' => 3);
    if ($b is shape('a' => this::T)) {}
  }
}
