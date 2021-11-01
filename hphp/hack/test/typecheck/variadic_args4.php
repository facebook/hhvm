<?hh

class C1 {
  public function meth(mixed ...$args): void {}
}

class C2 extends C1 {
  public function meth(mixed $x = null): void {}
}
