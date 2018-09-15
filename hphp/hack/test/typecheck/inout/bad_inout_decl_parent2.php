<?hh // strict

class C1 {
  public function foo(string $x, int $y): void {}
}

class C2 extends C1 {
  public function foo(string $x, inout int $y): void {}
}
