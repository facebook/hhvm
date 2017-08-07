<?hh // strict

abstract class Foo {
  abstract const type T;
  public function get(): this::T {
    // UNSAFE
  }
  public function instance(): this::T {
    static $instance = $this->get();
    return $instance;
  }

}

class Bar extends Foo {
  const type T = int;
}

class Baz extends Foo {
  const type T = num;
}
