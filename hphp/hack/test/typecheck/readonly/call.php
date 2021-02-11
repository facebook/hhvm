<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public int $prop;
  public function __construct() {
    $this->prop = 1;
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
