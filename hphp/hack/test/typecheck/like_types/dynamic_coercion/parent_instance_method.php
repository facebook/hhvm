<?hh // partial

class C {
  public function f(int $i): void {}
}
class D extends C {
  public function g(dynamic $d): void {
    parent::f($d);
  }
}
