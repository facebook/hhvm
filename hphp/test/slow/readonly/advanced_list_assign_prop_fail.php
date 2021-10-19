<?hh
class Baz {
}
class Foo {
  public int $prop;
  public readonly vec<Foo> $bar = vec[];
  public Baz $baz;
  public function __construct() {
    $this->prop = 1;
    $this->baz = new Baz();
  }

}
<<__EntryPoint>>
function main(): void{
  $y = readonly Vector{ new Foo(), new Baz()  };
  $z = new Foo();
  list($z->bar[], $z->baz) = $y;
  echo "Done!\n";
}
