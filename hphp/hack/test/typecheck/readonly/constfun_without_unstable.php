<?hh
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

<<__ConstFun>>
  function foo(<<__ConstFun>> (function(): void) $f,
  // error, constfun only works on functions
  <<__ConstFun>> int $x): void {
}
