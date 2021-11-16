<?hh

trait MyTrait {
  public function foo(): void { $this->bar(); }
  final private function bar(): void {}
}

class MyClass {
  use MyTrait;
  private function bar(): void {} // should error, clobbering trait internals
}
