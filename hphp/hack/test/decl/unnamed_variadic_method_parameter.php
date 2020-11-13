<?hh // strict

class C {
  public function f(int $foo, ...): void {
    throw new Exception();
  }
}
