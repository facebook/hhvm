<?hh // strict

class Test {
  public function __construct(public int $val) {}
}

class Another {
  <<__Rx, __Mutable>>
  public async function foo(int $x, int $y): Awaitable<void> {}
}
