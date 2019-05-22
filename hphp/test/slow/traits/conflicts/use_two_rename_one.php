<?hh

trait T1 { public function f() { echo "Dog"; } }

trait T2 { public function f() { echo "Cat"; } }

class C {
  use T1, T2;
  public function f() = T1::f;
  public function g() = T2::f;
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->f();
$c->g();
}
