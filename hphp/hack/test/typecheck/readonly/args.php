<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public int $prop;
  public function __construct() {
    $this->prop = 1;
  }

  public readonly function takes_mutable(Foo $other) : void {
    $other->prop = 4;
  }
  public readonly function takes_readonly(
    readonly Foo $other,
    Foo $mutable
  ) : void {
  }
}
function takes_mutable(Foo $x) : void {
  $x->prop = 4;
}

function takes_readonly(readonly Foo $other, Foo $mutable) : void {
}

function test(): void {
  $x = readonly new Foo();

  // error, $ro_foo is readonly
  takes_mutable($x);
  $x->takes_mutable($x);

  // ok
  takes_readonly($x, new Foo());
  $x->takes_readonly(new Foo(), new Foo());
}
