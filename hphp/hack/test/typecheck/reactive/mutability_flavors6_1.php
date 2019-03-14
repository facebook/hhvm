<?hh // partial

class A {}

<<__Rx>>
function f(bool $x): void {
  // ERROR
  $a = $x ? new A() : \HH\Rx\mutable(new A());
}
