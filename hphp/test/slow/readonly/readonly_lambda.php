<?hh
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
    $x->prop = 4; // ok
    $z->prop = 4; // error, $z is readonly
    return;
  };
  $z->prop = 4;// ok
  return new Baz();
}
