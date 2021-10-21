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

function test_try(bool $b): void {
  $l = readonly new Foo();
  try {
    $l = new Foo();
  } catch (Exception $e) {
    $l = new Foo();
  }
  $l->prop = 4; // ok, $l is mutable in all cases

  $v = readonly new Foo();
  try {
    $v = new Foo();
  } catch (Exception $e) {
  }
  $v->prop = 2; // error, $v could be readonly
}
