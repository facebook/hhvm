<?hh // strict
// Test for interface trying to implement

interface IFace<T> {
  public function foo(): T;
}

interface IFace2<T> implements IFace<T> {
  public function foo(): T;
  public function bar(): T;
}
