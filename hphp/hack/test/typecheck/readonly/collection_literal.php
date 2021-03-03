<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Bar {}
class Foo {
  public int $prop;
  public readonly Bar $ro;
  public Bar $not_ro;
  public function __construct() {
    $this->prop = 1;
    $this->ro = new Bar();
    $this->not_ro = new Bar();
  }
  public function set(int $y) : void {
    $this->prop = $y;
  }

  public readonly function get() : int {
    return 4;
  }
}


function test(): void {
  $z = vec[readonly new Foo(), new Foo()];
  $a = $z[0];
  $a->prop = 4; // error $a is readonly
  $b = $z[1];
  $b->prop = 5; // error $b is readonly

  $w = Vector { readonly new Foo(), new Foo() };
  $a = $w[0];
  $a->prop = 4; // error $a is readonly
  $b = $w[1];
  $b->prop = 5; // error $b is readonly

  // Note that the key is aways an arraykey, which is primitive/mutable
  $m = Map { "a" => readonly new Foo(), "b" => new Foo() };
  $a = $m["a"];
  $a->prop = 4; // error $a is readonly
  $b = $m["b"];
  $b->prop = 5; // error $b is readonly
}
