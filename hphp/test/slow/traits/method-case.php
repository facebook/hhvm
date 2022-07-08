<?hh
trait T1 {
  public function A(): void {
    echo "A\n";
  }
}
trait T2 {
  public function a(): void {
    echo "a\n";
  }
}
class C {
  use T1, T2;
}

<<__EntryPoint>> function test(): void {
  $c = new C();
  $c->A();
  $c->a();
}
