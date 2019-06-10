<?hh // strict

class Foo {

  public function bar(): int {
    return ($this->prop)();
  }

  public function prop(): int {
    return 42;
  }
}
