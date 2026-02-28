<?hh
class B {
  public function a(): void {
    echo "a\n";
  }
}
class C extends B {
  public function A(): void {
    echo "A\n";
  }
}

<<__EntryPoint>> function testcase(): void {
  $c = new C();
  $c->a();
  $c->A();
}
