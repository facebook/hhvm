<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Foo {
  public int $prop;
  public function __construct() {
    $this->prop = 1;
  }
}

function readonly_return(): readonly Foo {
  return new Foo();
}

function bad_return(): Foo {
  return readonly new Foo();
}
