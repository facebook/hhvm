<?hh

class C {
  public static function f<reify T>() {
    var_dump("hi");
  }
}

C::f();
