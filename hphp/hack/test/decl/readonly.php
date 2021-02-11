<?hh // strict
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
}

async function returns_readonly() : readonly Awaitable<Foo> {
  return new Foo();
}

async function returns_normal(): Awaitable<Foo> {
  return new Foo();
}

class Bar {
  public readonly Foo $x;

  public function __construct(
    public readonly Foo $y,
    ) {
    $this->x = new Foo();
  }
  public readonly function getFoo() : void {
    $f = /* <<readonly>> */  (Foo $y) ==> {return $y;};
    $z = readonly function(Foo $f)  : void {
    };
  }

}
