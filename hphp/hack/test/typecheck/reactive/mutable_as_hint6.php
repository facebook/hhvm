<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__NonRx("what?")>>
  public function m(<<__OwnedMutable>> A $a): void {
  }
}
