<?hh // partial

class C1 {
  public function meth(string ...$args): void {}
}

class C2 extends C1 {
  public function meth(int $x = 0, string ...$args): void {}
}
