<?hh // strict

class C {}

<<__Rx, __MutableReturn>>
function make(): C {
  return new C();
}

class A {
  <<__Rx, __MutableReturn>>
  public function f3(): C {
    // OK
    return make();
  }
}
