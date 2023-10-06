<?hh

<<file:__EnableUnstableFeatures('nameof_class')>>

class C {
  public static function test(): void {
    var_dump(nameof static);
  }
}
class D extends C {}

<<__EntryPoint>>
function main(): void {
  D::test();
}
