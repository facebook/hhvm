<?hh // partial
trait T1 {
  public function foo(): void {}
}
trait T2 {
  public function foo(): void {}
}

class C {
  use T1, T2;
}
