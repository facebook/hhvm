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
}


function test(readonly vec<Foo> $vec): void {
  foreach($vec as $val) {
    $val->prop = 3; // error, $val is readonly
  }
  // This will error since $val is already assigned a readonly value.
  // TODO: make this not error
  $val = new Foo();
}
