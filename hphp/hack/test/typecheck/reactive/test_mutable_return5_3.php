<?hh // strict
class C {}


function make(): C {
  return new C();
}

class A {

  public function f3(): C {
    // OK
    return make();
  }
}
