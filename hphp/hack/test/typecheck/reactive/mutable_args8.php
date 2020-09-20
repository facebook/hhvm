<?hh // strict

class A {
  <<__Rx, __Mutable>>
  public function f(<<__MaybeMutable>>A $a): void {
  }
}

<<__Rx>>
function g(): void {
  $a = \HH\Rx\mutable(new A());
  // ERROR: cannot borrow mutable value more than once
  $a->f($a);
}
