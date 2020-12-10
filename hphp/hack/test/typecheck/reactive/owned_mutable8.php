<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  // ERROR
  public function f(<<__OwnedMutable>> A $a): void {
  }
}
