<?hh

abstract final class Foo {
  public static function bar() {}
}

class C<T as Foo> {}
