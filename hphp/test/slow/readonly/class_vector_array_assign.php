<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Baz {
}
class Foo {
  public int $prop;
  public readonly vec<Foo> $bar = vec[];
  public static readonly Vector<Foo> $static_vec = Vector{};
  public readonly Baz $baz;
  public function __construct() {
    $this->prop = 1;
    $this->baz = new Baz();
  }

}
<<__EntryPoint>>
function main(): void {
  $y = readonly Foo::$static_vec;
  (Foo::$static_vec)[] = new Foo();
  echo "Done\n";
}
