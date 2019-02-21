<?hh // partial

class A {}

function g(A $a): void {
}

<<__Rx>>
function f(): void {
  $a = \HH\Rx\mutable(new A());
  // OK
  $b = \HH\Rx\move($a);
  // ERROR
  g($a);
}
