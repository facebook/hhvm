<?hh
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
  $x = readonly new Foo();
  $x->bar[] = new Foo(); // error, $x is readonly
  echo "Done\n";
}
