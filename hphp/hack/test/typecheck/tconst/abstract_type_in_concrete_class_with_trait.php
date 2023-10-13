<?hh

abstract class Foo {
  abstract const int NUM;
}

trait TFoo {
  require extends Foo;
}

final class Bar extends Foo {
  use TFoo;
  final public static function get(): int {
    return 1;
  }
}
