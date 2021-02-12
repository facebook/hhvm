<?hh // strict
class Test {
  public function __construct(public int $val) {}
}

class Another {

  public async function foo(int $x, int $y): Awaitable<void> {}
}
