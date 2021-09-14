<?hh

<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public static readonly vec<int> $cow_c = vec[1,2,3];
  public static readonly Vector<int> $c = Vector {1,2,3};
}

<<__EntryPoint>>
function test(): void {
  Foo::$cow_c[1] = 2;
  Foo::$c[1] = 2;
}
