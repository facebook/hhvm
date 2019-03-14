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
  // OK
  $b = \HH\Rx\mutable(g());
  $b->a = 5;

  // OK
  \HH\Rx\mutable(g())->a = 10;

  // OK
  $b->f(42);
}
