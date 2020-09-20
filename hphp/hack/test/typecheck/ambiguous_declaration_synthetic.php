<?hh // partial

class C {
  public function foo(): void {}
}
trait T {
  require extends C;
}
trait T2 {
  require extends C;
}
class D extends C {
  use T, T2;
}
