<?hh // strict
class C {}


function make(): C {
  return new C();
}


function make2(int $a): C {
  return new C();
}

class A {


  public function f7(): C {
    // OK
    return 1 |> make2($$);
  }
}
