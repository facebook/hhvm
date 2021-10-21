<?hh
class Bar {
  public int $prop;
}
class Foo {
  public int $prop;
  public readonly Bar $ro;
  public Bar $not_ro;
  public readonly vec<Bar> $ro_vec;
  public function __construct() {
    $this->prop = 1;
    $this->ro = new Bar();
    $this->not_ro = new Bar();
    $this->ro_vec = vec[];
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

 $x->ro_vec[] = readonly new Bar();
 echo "Done\n";
}
