<?hh // partial

abstract final class Foo {
  public static function bar() {}
}

class C {
  public function takes_complex_foo<T as Vector<array<Awaitable<Foo>>>>(
    T $x
  ): void {}
}
