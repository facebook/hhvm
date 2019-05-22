<?hh

trait T {
  public function f() {
    echo "T::f\n";
  }

  public function k() {
    echo "T::k\n";
  }
}

trait R {
  public function f() {
    echo "R::f\n";
  }

  public function g() {
    echo "R::g\n";
  }
}

class C {
  use T, R;
  public function f() = T::f;
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->f();
$c->g();
$c->k();
}
