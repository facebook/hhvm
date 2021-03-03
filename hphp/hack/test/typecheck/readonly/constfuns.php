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

function const_eval<T>(<<__ConstFun>> (function(): T) $f) : T {
    return $f();
}



function test() : void {
  $x = new Foo();

  $y = <<__ConstFun>> () : readonly Foo  ==> {
    return $x;
  };

  $z = () : Foo ==> {
    return $x;
  };
  const_eval($y);
  const_eval($z); // $z is not a constfun, error


}
