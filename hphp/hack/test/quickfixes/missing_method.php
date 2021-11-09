<?hh

interface IFoo {
  public function bar(): void;
}

class Foo implements IFoo {
  public function otherMethod(): void {}
}
