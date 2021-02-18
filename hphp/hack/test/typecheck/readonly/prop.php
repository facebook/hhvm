<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Bar {}
class Foo {
  public int $prop;
  public readonly Bar $ro;
  public function __construct() {
    $this->prop = 1;
    $this->ro = new Bar();
  }
  public function set(int $y) : void {
    $this->prop = $y;
  }

  public readonly function get() : int {
    return 4;
  }
}


function test(Foo $x): void {
  $y = $x->ro; // needs to be wrapped in a readonly
  $ro_prop = readonly $x->ro; // good
  $z = readonly new Foo();
  $a = $z->ro; // since $z is known to be readonly,
  // $z->ro is therefore always readonly, and doesn't need to be cast.
}
