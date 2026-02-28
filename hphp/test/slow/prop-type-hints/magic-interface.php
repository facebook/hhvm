<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  private static Traversable $x;
  public static function set() :mixed{
    self::$x = __hhvm_intrinsics\launder_value(vec[]);
  }
  public static function get() :mixed{
    $x = self::$x;
    $x[] = 456;
    return $x;
  }
}

<<__EntryPoint>> function test(): void {
  A::set();
  var_dump(A::get());
}
