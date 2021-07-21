<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Foo {
  public function __construct(int $prop = 4) {}
  public readonly function test(): void {
    $this->prop = 4;
  }
}

<<__EntryPoint>>
  function main() : void {
    $x = new Foo(4);
    $x->test();
  }
