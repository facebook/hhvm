<?hh

abstract final class Foo {
  public static function bar(): void {}
}

function takes_complex_foo(
  Vector<varray<Awaitable<Foo>>> $foo
): void {}
