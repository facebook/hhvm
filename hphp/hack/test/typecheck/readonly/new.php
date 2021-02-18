<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Bar {}
class Foo {
  public int $prop;
  public readonly Bar $ro;
  public function __construct(public Foo $x, public readonly Foo $y) {
    $this->prop = 1;
    $this->ro = new Bar();
  }

}


function test(Foo $x): void {
  $t = new Foo($x, $x);
  // error, $x is readonly
  $v = new Foo(readonly $x, $t);
}
