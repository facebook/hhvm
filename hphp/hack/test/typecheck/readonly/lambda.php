<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Baz {
}
class Foo {
  public int $prop;
  public readonly Baz $baz;
  public function __construct() {
    $this->prop = 1;
    $this->baz = new Baz();
  }

}
function returns_readonly(Foo $z) : readonly Baz {
  $f =
    readonly (Foo $x, readonly Foo $y) ==> {
    $z->prop = 4; // error, $z is readonly
    $x->prop = 4; // ok
    $y->prop = 4; // obviously not ok
    return;
  };
  $z->prop = 4;// ok
  return new Baz();
}
