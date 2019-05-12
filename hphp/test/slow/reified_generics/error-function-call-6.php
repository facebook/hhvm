<?hh

class C {
  public static function f<reify T>() {
    var_dump("hi");
  }
}
<<__EntryPoint>> function main(): void {
C::f();
}
