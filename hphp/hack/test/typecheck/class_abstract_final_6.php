<?hh // partial

abstract final class Foo {
  public static function bar() {}
}

function takes_complex_foo<T as Vector<varray<Awaitable<Foo>>>>(
  T $x
): void {}
