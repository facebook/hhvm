<?hh

class A {
  // ERROR
  public function f(<<__RxOfScope>>int $a): void {
  }
}
