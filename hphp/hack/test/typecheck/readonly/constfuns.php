<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Baz {
}
class Foo {
  public int $prop;
  public Baz $baz;
  public function __construct() {
    $this->prop = 1;
    $this->baz = new Baz();
  }

}
// TODO: the first readonly in front of function typehint should be optional
function const_eval<T>(readonly (readonly function(): readonly T) $f) : readonly T {
    return readonly $f();
}



function test() : void {
  $x = new Foo();

  $y = readonly () : readonly Foo  ==> {
    $x->prop = 4; // error $x is readonly here
    return $x;
  };

  $w = readonly function() : readonly Foo use($x) {
    $x->prop = 5; // error, $x is readonly here
    return $x;
  };

  $z = () : Foo ==> {
    $x->prop = 4; // no error
    return $x;
  };
  readonly const_eval($w);
  readonly const_eval($y);
  readonly const_eval($z); // $z is not a constfun, error


}
