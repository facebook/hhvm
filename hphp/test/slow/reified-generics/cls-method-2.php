<?hh

class C {
  public static function f<reify T>() :mixed{
    var_dump("hi");
  }
  public static function g() :mixed{
    self::f<int>();
  }
}
<<__EntryPoint>> function main(): void {
C::g();
}
