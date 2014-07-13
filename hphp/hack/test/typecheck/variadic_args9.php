<?hh

class C1 {
  public function meth(string ...$args): void {}
}

class C2 extends C1 {
  public function meth(int ...$args): void {}
}
