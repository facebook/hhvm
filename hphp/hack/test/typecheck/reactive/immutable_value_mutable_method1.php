<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx, __Mutable>>
  public function f(): void {
  }
}

<<__Rx>>
function f(): void {
  $a = new A();
  $a->f();
}
