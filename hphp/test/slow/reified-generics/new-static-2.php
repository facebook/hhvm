<?hh

class C<reify Ta, reify Tb> {
  public static function f() :mixed{
    new static();
  }
}
<<__EntryPoint>> function main(): void {
C::f();
}
