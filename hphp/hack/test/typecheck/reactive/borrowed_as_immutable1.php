<?hh // strict

class A {
  public ?int $value;
}

<<__Rx>>
function f(<<__Mutable>>A $v): A {
  // ERROR, cannot return borrowed as immutable
  return $v;
}
