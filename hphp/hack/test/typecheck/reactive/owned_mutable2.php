<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
class A {
  // ERROR
  <<__OwnedMutable>>
  public function f(): void {
  }
}
