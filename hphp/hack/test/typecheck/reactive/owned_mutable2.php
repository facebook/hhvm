<?hh // partial
class A {
  // ERROR
  <<__OwnedMutable>>
  public function f(): void {
  }
}
