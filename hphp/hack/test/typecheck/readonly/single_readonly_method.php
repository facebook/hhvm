<?hh // strict

class Foo {
  public function get(): Foo {
    return new Foo();
  }
  public readonly function test(): void {
    $x = $this->get();
  }
}
<<__EntryPoint>>
  function test() : void {
    $x = new Foo();
    $x->test();
  }
