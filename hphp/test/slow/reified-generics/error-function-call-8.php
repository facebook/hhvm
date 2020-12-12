<?hh

trait T {
  public static function f<reify T>() {
    var_dump("hi");
  }
}

class C {
  use T;
}
<<__EntryPoint>> function main(): void {
C::f();
}
