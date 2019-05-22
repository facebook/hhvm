<?hh

trait T1 { public function f() { echo "Dog\n"; } }

class C {
  use T1;
  public function g() = T1::f;
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->g();
$c->f();
}
