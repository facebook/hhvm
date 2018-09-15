<?hh // strict

class A {
  <<__Rx>>
  public function f(<<__Mutable>>A $a, <<__Mutable>>A $b): void {
  }
}

<<__Rx>>
function g(A $v): void {
  $a = \HH\Rx\mutable(new A());
  // ERROR: cannot borrow mutable value more than once
  $v->f($a, $a);
}
