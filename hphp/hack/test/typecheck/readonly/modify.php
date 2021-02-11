<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public int $prop;
  public function __construct() {
    $this->prop = 1;
  }
}
function test(): void {
  $x = readonly new Foo();
  // Error $x is readonly
  $x->prop = 4;
}

function test2(): void {
  $x = new Foo();
  // Error, $x is reassigned to readonly value
  // (TODO: improve this to not error)
  $x = readonly new Foo();
}
