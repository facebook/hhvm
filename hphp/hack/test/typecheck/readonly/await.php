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

async function foo(): readonly Awaitable<Foo> {
  return new Foo();
}


async function test(): Awaitable<void> {
  $x = await readonly foo();
  $x->prop = 4;

}
