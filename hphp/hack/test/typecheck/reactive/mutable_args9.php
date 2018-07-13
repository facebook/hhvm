<?hh // strict

class A {
  <<__Rx, __MaybeMutable>>
  public function f(<<__Mutable>>A $a): void {
  }
}

<<__Rx>>
function g(): void {
  $a = \HH\Rx\mutable(new A());
  // ERROR: cannot borrow mutable value more than once
  $a->f($a);
}
