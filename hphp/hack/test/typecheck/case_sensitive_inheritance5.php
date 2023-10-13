<?hh
class A {
  public function FOO(): void {}
}
trait T {
  public function foO(): void {}
  public function bar(): int {
    return 0;
  }
}
class B extends A {
  use T;
  public function FOO(): void {}
}
