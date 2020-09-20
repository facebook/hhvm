<?hh // partial

class A {}

<<__Rx>>
function f(bool $x): void {
  $a = \HH\Rx\mutable(new A());
  if ($x) {
    // ERROR
    $a = new A();
  }
  else {
    $a = \HH\Rx\mutable(new A());
  }
}
