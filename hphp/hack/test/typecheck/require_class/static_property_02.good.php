<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  static public int $x = 1;

  public static function foo(): int { return C::$y; }
}

final class C  {
  use T;

  static public int $y = 2;

  public static function bar(): int { return C::$x; }
}
