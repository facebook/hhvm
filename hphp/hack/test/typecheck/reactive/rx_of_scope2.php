<?hh

class A {
  // ERROR
  <<__RxOfScope>>
  public function f(): void {
  }
}
