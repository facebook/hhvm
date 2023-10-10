<?hh

class Foo {

  public function __construct(
    private (function (): int) $prop,
  ) {}

  public function bar(): int {
    return ($this->prop)();
  }
}

<<__EntryPoint>> function main(): void {
  $foo = new Foo(() ==> { var_dump('hello'); return 42; });
  var_dump($foo->bar());
}
