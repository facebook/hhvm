<?hh // partial

class A {
  public ?int $a;
  <<__Rx, __Mutable>>
  public function f(int $a) {
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
  $b->a = 5;
}
