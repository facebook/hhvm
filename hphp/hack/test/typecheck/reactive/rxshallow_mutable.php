<?hh // strict

class C1 {}

<<__RxShallow>>
function f(<<__Mutable>>C $a): int {
  return 1;
}

class C {
  <<__RxShallow, __Mutable>>
  public function f(<<__Mutable>>C $a): int {
    return 1;
  }
}
