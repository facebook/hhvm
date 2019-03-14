<?hh // partial

class A {}

<<__Rx>>
function f(bool $x): void {
  // ERROR
  if ($x) {
    $a = new A();
  }
  else {
    $a = \HH\Rx\mutable(new A());
  }
}
