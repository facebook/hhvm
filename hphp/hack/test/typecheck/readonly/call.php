<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public int $prop;
  public (function(): void) $fprop;
  public function __construct() {
    $this->prop = 1;
    $this->fprop = () ==> {};
  }
  public function set(int $y) : void {
    $this->prop = $y;
  }

  public readonly function get() : int {
    return 4;
  }
}


function test(): void {
  $x = readonly new Foo();
  $y = $x->get(); // ok
  $x->set(6); // error, can't call mutable function on readonly
}

// TODO: error against the $g typehint in nastcheck
function test_closure(readonly (readonly function() : void) $f, readonly (function(): void) $g) : void {
  $f(); // ok
  $g(); // error $g can't be called
  $x = readonly new Foo();
  ($x->fprop)();
}
