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
    <<__Const>> (Foo $x, Foo $y) ==> {
    $z->prop = 4; // error, $z is readonly
    $x->prop = 4; // error, $x is readonly
    $y->prop = 4; // error, $y is readonly
    return;
  };
  $z->prop = 4;// ok
  return new Baz();
}
