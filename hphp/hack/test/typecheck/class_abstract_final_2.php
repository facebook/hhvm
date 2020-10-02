<?hh // partial

abstract final class Foo {
  public static function bar() {}
}

class C {
  public static function takes_complex_foo(
    Vector<varray<Awaitable<Foo>>> $foo
  ) {}
}
