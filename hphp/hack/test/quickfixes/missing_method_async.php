<?hh

interface IFoo {
  public function genBar(): Awaitable<int>;
}

class Foo implements IFoo {
  public function otherMethod(): void {}
}
