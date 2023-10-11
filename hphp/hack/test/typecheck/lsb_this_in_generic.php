<?hh

class A {
  <<__LSB>> private static vec<this> $v = vec[];

  public static function foo(): vec<this> {
    return static::$v;
  }
}

class B extends A {
}

class C extends B {
}

class D {
  public static function bar(): void {
    self::baz(A::foo());
    self::qux(B::foo());
  }

  private static function baz(vec<A> $a): void {
  }

  private static function qux(vec<B> $b): void {
  }
}
