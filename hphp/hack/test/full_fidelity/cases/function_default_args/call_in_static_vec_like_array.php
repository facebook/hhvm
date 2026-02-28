<?hh

class C {
  public static function f() {
    return "lol";
  }
}

function f($x = vec[C::f()]): void{}
