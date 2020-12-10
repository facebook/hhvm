<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class Test {
  public function __construct(public int $val) {}
}

<<__Rx>>
async function foo(<<__Mutable>>Test $x, int $y): Awaitable<void> {}
