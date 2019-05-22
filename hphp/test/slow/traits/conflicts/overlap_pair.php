<?hh

trait T {
  public function f() {
    echo "T::f\n";
  }
}

trait R {
  public function g() {
    echo "R::g\n";
  }
}

class C {
  use T, R;
  public function f() = R::g;
  public function g() = T::f;
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->f();
$c->g();
}
