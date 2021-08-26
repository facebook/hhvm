<?hh
<<file:__EnableUnstableFeatures('readonly')>>
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
  $rl->prop = 4; // ok, $l is mutable in all cases

  try {
    $v = readonly new Foo();
  } catch (Exception $e) {
  } finally {
    $v = new Foo();
  }
  $v->prop = 4; // ok, $v is mutable due to finally

  try {
    $v = new Foo();
  } catch (Exception $e) {
    $v = readonly new Foo();
  } finally {
    $v->prop = 4; // error, $v could be readonly here
  }
}
