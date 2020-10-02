<?hh // partial

abstract final class Foo {
  public static function bar() {}
}

function takes_complex_foo(
  Vector<varray<Awaitable<Foo>>> $foo
) {}
