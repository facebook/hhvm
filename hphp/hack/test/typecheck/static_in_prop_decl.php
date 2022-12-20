<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  const A = 5;
  public static int $x = static::$x;
  public static vec<int> $y = vec[static::$x];
  public static function test():int {
    return static::$x;
  }
}
