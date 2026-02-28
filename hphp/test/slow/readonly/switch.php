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

function test_switch(int $b) :mixed{
  $x = new Foo();
  switch($b) {
    case 0:
      $x = new Foo();
    case 1:
      $x = new Foo();
    case 2:
      $y = 4;
    default:
      $x = readonly new Foo();
  }
  $x->prop = 4; // error, $x could be readonly


}
