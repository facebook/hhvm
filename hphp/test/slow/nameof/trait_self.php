<?hh

<<file:__EnableUnstableFeatures('nameof_class')>>

trait T {
  public static function test(): void {
    var_dump(nameof self);
  }
}
class C { use T; }
class D { use T; }

<<__EntryPoint>>
function main(): void {
  C::test();
  D::test();
}
