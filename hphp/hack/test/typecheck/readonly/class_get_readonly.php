<?hh
class Bar {
  public int $prop = 1;
}
class Foo {
  public static readonly ?Bar $ro_static = null;
  public int $prop;
  public readonly Bar $ro;
  public Bar $not_ro;
  public readonly vec<Bar> $ro_vec;
  public readonly Vector<Bar> $ro_vector;
  public vec<Bar> $not_ro_vec ;
  public Vector<Bar> $not_ro_vector;
  public function __construct() {
    $this->prop = 1;
    $this->ro = new Bar();
    $this->not_ro = new Bar();
    $this->ro_vector = Vector<Bar> {};
    $this->ro_vec = vec[];
    $this->not_ro_vec = vec[];
    $this->not_ro_vector = Vector<Bar> {};
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
  $y = Foo::$ro_static;
}
