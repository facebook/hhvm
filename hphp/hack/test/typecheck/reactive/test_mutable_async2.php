<?hh // strict

class Test {
  public function __construct(public int $val) {}
}

<<__Rx>>
async function foo(<<__Mutable>>Test $x, int $y): Awaitable<void> {}
