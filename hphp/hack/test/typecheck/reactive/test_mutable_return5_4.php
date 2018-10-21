<?hh // strict

class C {}

<<__Rx, __MutableReturn>>
function make(): C {
  return new C();
}

<<__Rx, __MutableReturn>>
function make2(int $a): C {
  return new C();
}

class A {

  <<__Rx, __MutableReturn>>
  public function f5(bool $b): C {
    // ERROR
    return $b ? make() : make2(42);
  }
}
