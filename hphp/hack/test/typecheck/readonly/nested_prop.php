<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Foo {
  public function __construct(
    public int $prop,
  ){}
}

class Bar {

  public function __construct(
    public readonly Foo $foo
  ) {}
}


function test(Bar $b) : void {
  $b->foo->prop = 4;
}
