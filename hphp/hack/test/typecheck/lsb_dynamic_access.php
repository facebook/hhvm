<?hh
<<file: __EnableUnstableFeatures('class_type')>>
class A {
  <<__LSB>> public static int $x = 1;
  <<__LSB>> public static ?this $y = null;
}

class B {
  public static function foo(class<A> $cls): int {
    return $cls::$x;
  }

  public static function bar<T as A>(class<T> $cls): ?T {
    return $cls::$y;
  }
}
