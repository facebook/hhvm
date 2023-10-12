<?hh // strict

class C1 {
  public function foo((function(inout string): void) $f): void {}
}

class C2 extends C1 {
  public function foo((function(inout arraykey): void) $f): void {}
}
