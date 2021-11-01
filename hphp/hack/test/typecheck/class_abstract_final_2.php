<?hh

abstract final class Foo {
  public static function bar(): void {}
}

class C {
  public static function takes_complex_foo(
    Vector<varray<Awaitable<Foo>>> $foo
  ): void {}
}
