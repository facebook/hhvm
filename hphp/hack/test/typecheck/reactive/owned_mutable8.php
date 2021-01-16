<?hh
class A {
  // ERROR
  public function f(<<__OwnedMutable>> A $a): void {
  }
}
