<?hh // strict

class A {
  // OK
  <<__Rx>>
  public function f(<<__MaybeMutable>>A $a): int {
    return 1;
  }
}

<<__Rx>>
function f(<<__MaybeMutable>>A $a): int {
  return 1;
}
