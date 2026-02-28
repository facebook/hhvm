<?hh
interface IFoo {
  public function foo(): int;
}

interface IFooChild extends IFoo {
  public function Foo(): float; // hh should error
}
