<?hh

class Foo<+T> {
  public function bar<Tu as T>(Tu $x): void {}
}
