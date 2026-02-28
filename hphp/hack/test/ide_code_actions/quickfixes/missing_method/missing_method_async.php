<?hh

interface IFoo {
  public function genBar(): Awaitable<int>;
}

class Foo implements IFoo {
  //                  ^ at-caret
  public function otherMethod(): void {}
}
