<?hh // strict

class A {
}

<<__Rx>>
function f(<<__Mutable>>A $a, <<__Mutable>>A $b): void {
}

<<__Rx>>
function g(): void {
  $a = \HH\Rx\mutable(new A());
  // ERROR: cannot borrow mutable value more than once
  f($a, $a);
}
