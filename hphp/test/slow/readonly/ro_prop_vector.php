<?hh
class Bar {
  public int $prop;
}
class Foo {
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
    $this->ro_vector = Vector {};
    $this->ro_vec = vec[];
    $this->not_ro_vec = vec[];
    $this->not_ro_vector = Vector {};
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
 $x->ro_vec[] = new Bar(); // ok
 $x->not_ro_vector[] = new Bar(); // ok
 $x->ro_vector[] = new Bar(); // not ok, Vector must be mutable
}
