<?hh
class Bar {}
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


function test(readonly shape('a' => Foo, 'b' => Foo) $x, readonly (Foo, Foo) $y): void {
  shape('a' => $a, 'b' => $b) = $x;
  $a->prop = 4; // error $a is readonly
  $b->prop = 4; // error $b is readonly
  tuple($c, $d) = $y;
  $c->prop = 4; // error $c is readonly
}
