<?hh

abstract final class Foo {
  public static function bar(): void {}
}

class C {
  public function takes_complex_foo<T as Vector<varray<Awaitable<Foo>>>>(
    T $x
  ): void {}
}
