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


<<__EntryPoint>>
function test(): void {
   $y = readonly Vector{ new Foo(), new Foo() };
   $w = readonly Vector{$y, $y};
   list(list($w[0][1], $b), $_) = $w;
}
