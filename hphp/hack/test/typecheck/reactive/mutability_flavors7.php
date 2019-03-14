<?hh // partial

class A {}

<<__Rx>>
function f(bool $x): void {
  if ($x) {
    // ERROR
    $a = new A();
    $z = new A();
  }
  else {
    $a = \HH\Rx\mutable(new A());
    $z = \HH\Rx\freeze($a);
  }
}
