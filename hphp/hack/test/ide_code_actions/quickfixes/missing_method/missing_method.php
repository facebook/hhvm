<?hh

interface IFoo {
  public function bar(): void;
  public function baz(): void;
}

class Foo implements IFoo {
                   // ^ at-caret
  public function otherMethod(): void {}
}
