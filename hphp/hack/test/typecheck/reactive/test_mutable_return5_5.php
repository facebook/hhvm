<?hh // strict
class C {}


function make(): C {
  return new C();
}


function make2(int $a): C {
  return new C();
}

class A {


  public function f6(): C {
    // ERROR
    return make() ?? make2(42);
  }
}
