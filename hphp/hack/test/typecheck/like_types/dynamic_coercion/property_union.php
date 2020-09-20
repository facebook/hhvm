<?hh

class C {
  public int $prop = 4;
}

class D {
  public string $prop = "3";
}

function f(bool $b, dynamic $d): void {
  $x = $b ? new C() : new D();
  // $x : C|D
  $x->prop = $d;
}
