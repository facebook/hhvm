<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public int $prop;
  public function __construct() {
    $this->prop = 1;
  }
}
function test(readonly Foo $z, readonly Foo $y): void {
  $x = new Foo();
  $z = $y;
  $z->prop = 4;
  $t = readonly $x;

}

class Test {
  public function bar(readonly Foo $f): void {
    $f->prop = 4;
  }
}
