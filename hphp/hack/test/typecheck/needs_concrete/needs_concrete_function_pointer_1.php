<?hh

abstract class A {
  abstract public static function hook(): int;

  <<__NeedsConcrete>>
  public static function m(): int {
    return static::hook();
  }

  <<__NeedsConcrete>>
  public static function g<T>(T $_): int {
    return static::hook();
  }

  public static function forward(): void {
    $_ = static::m<>;
  }
}

final class C extends A {
  public static function hook(): int {
    return 1;
  }
}

function f(): void {
  A::m();
  $_ = A::m<>;
  $_ = A::g<int>;
  $_ = C::m<>;
}
