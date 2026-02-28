<?hh

class C1 {
  public function foo((function(string, inout int): void) $f): void {}
}

class C2 extends C1 {
  public function foo((function(string, int): void) $f): void {}
}
