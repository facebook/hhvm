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
  public function f6(): C {
    // ERROR
    return make() ?? make2(42);
  }
}
