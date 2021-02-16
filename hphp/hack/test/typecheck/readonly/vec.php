<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public int $prop;
  public function __construct() {
    $this->prop = 1;
  }
}



async function test(
  readonly vec<Foo> $v,
  readonly dict<int, Foo> $d
): Awaitable<void> {
  $z = $v[0];
  $z->prop = 5;
  $w = $d[2];
  $w->prop = 4;
}
