<?hh

<<file:__EnableUnstableFeatures('nameof_class')>>

trait T {
  public static function test(): void {
    var_dump(nameof parent);
  }
}

class C {}
class D extends C { use T; }
class E extends D { use T; }

<<__EntryPoint>>
function main(): void {
  D::test();
  E::test();
}
