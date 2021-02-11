<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public int $prop;
  public function __construct() {
    $this->prop = 1;
  }
  public readonly function returns_readonly() : readonly Foo {
    return new Foo();
  }

  public function takes_mutable(Foo $other) : void {
    $other->prop = 4;
  }
}

function returns_readonly() : readonly Foo {
  return new Foo();
}

function test(): void {
  $x = readonly new Foo();
  // Error, must be cast to readonly
  $ro = returns_readonly();
  $ro = $x->returns_readonly();

  $better = readonly returns_readonly();
  $better = readonly $x->returns_readonly();
}
