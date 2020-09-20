<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  private static Traversable $x;
  public static function set() {
    self::$x = __hhvm_intrinsics\launder_value(vec[]);
  }
  public static function get() {
    $x = self::$x;
    $x[] = 456;
    return $x;
  }
}

<<__EntryPoint>> function test(): void {
  A::set();
  var_dump(A::get());
}
