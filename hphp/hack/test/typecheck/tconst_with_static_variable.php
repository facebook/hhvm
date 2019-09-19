<?hh // strict

abstract class Foo {
  abstract const type T;
  /* HH_FIXME[4336] */
  public function get(): this::T {
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
