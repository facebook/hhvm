<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx, __Mutable>>
  public function f(): void {
    // ERROR: aliasing mutable this
    $a = $this;
  }
}
