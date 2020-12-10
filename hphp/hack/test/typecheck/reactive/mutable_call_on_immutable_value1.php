<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx, __MutableReturn>>
  public static function make(): A {
    return new A();
  }

  <<__Rx, __Mutable>>
  public function g():void {
  }
}

<<__Rx>>
function f(): void {
  (A::make())->g();
}
