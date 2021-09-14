<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Baz {
}
class Foo {
  public int $prop;
  public readonly vec<Foo> $bar = vec[];
  public static readonly vec<Foo> $static_vec = vec[];
  public readonly Baz $baz;
  public function __construct() {
    $this->prop = 1;
    $this->baz = new Baz();
  }

}
<<__EntryPoint>>
function main(): void {
  $y = dict["a" => readonly new Foo()];
  $y["b"] = readonly new Foo();
  echo "Done\n";
}
