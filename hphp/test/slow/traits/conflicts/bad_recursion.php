<?hh

trait T1 {
  public function f() {
    echo "T1::f\n";
    $this->f();
  }
}

trait T2 {
  public function f() {
    echo "T2::f\n";
  }
}

class C {
  use T1, T2;
  public function g() = T1::f;
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->f();
$c->g();
}
