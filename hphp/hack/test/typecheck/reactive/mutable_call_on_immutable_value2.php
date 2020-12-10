<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx, __Mutable>>
  public function g():void {
  }
}

<<__Rx>>
function f(): void {
  (new A())->g();
}
