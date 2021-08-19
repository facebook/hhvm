<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Bar {
  public int $prop;
}
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
<<__EntryPoint>>
function test() : void {
 $x = new Foo();
 $x->ro = readonly new Bar();

 $x->ro->prop = 4;
}
