<?hh

enum E : int as arraykey {
  A = 0;
}

class B {
  public function f(int $x) : arraykey { return "s"; }
}

class C extends B {
  public function f(int $x) : E { return E::A; }
}

function f() : void {
  $c = new C();
  $y = $x ==> $c->f($x);
// ^ hover-at-caret
}
