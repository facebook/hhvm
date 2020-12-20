<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  public ?int $a;
  <<__Rx, __Mutable>>
  public function f(int $a)[rx]: void {
    $this->a = $a;
  }
}

<<__Rx, __MutableReturn>>
function g()[rx]: A {
  return new A();
}

<<__Rx>>
function f()[rx]: void {
  $b = g();
  // ERROR
  $b->a = 5;
}
