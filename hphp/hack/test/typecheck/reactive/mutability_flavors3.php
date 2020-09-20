<?hh // partial

class A {}

<<__Rx>>
function f(): void {
  $a = \HH\Rx\mutable(new A());
  // ERROR
  $a = \HH\Rx\freeze($a);
}
