<?hh // strict

class C {
  public function f(int $i): void {}
}
class D extends C {
  public function g(): void {
    parent::f(3);
  }
}
