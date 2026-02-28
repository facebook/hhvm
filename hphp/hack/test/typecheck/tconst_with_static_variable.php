<?hh

abstract class Foo {
  abstract const type T;
  public function get(): this::T {
    throw new Exception();
  }
  public function instance(): this::T {
    $instance = $this->get();
    return $instance;
  }

}

class Bar extends Foo {
  const type T = int;
}

class Baz extends Foo {
  const type T = num;
}
