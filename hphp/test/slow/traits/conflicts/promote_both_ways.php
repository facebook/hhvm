<?hh

trait T1 { public function f() { echo "Dog\n"; } }

trait T2 { private function g() { echo "Cat\n"; } }

class C {
  use T1, T2;
  private function f() = T1::f;
  public function g() = T2::g;
  public function exposer() {
    $this->f();
  }
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->exposer();
$c->g();
$c->f();
}
