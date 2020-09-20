<?hh // partial

class A {}

<<__Rx>>
function f(): void {
  $a = \HH\Rx\mutable(new A());
  // ERROR
  $a = new A();
}
