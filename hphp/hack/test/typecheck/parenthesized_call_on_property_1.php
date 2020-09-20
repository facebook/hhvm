<?hh // strict

class Foo {

  public function __construct(
    private (function (): int) $prop,
  ) {}

  public function bar(): int {
    return ($this->prop)();
  }
}
