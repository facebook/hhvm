<?hh

class C {
  public function f(int $arg): void {}
}

class D {
  public function f(string $arg): void {}
}

function f(bool $b, dynamic $d): void {
  $x = $b ? new C() : new D();
  // $x : C|D
  $x->f($d);
}
