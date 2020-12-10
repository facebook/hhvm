<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  public ?int $a;
  <<__Rx, __Mutable>>
  public function f(int $a): void {
    $this->a = $a;
  }
}

<<__Rx, __MutableReturn>>
function g(): A {
  return new A();
}

<<__Rx>>
function f(): void {
  $b = g();
  // ERROR
  $b->f(42);
}
