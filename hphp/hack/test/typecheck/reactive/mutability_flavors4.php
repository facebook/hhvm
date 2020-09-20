<?hh // partial

class A {}

<<__Rx>>
function f(): void {
  $a = 1;
  // ERROR
  $a = \HH\Rx\mutable(new A());
}
