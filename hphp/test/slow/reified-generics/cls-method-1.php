<?hh

class C {
  public static function f<reify T>() :mixed{
    var_dump("hi");
  }
}
<<__EntryPoint>> function main(): void {
C::f<int>();
}
