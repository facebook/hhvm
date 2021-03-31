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

function test2(bool $b): void {
  $x = new Foo();
  $x = readonly new Foo();
  $x->prop = 4;
  $x = new Foo();
  $x->prop = 4; // ok
  if ($b) {
    $y = readonly new Foo();
    $x = readonly new Foo();
  } else {
    $y = new Foo();
  }
  $y->prop = 4; //error, $y (can be) readonly from branch 1
  $x->prop = 4; // error, $x (can be) readonly from branch 1
}
