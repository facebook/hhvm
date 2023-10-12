<?hh // strict
class Foo {
  public int $x = 4;
  public readonly function getInt(): int {
    return $this->x ?as int;
  }
}
<<__EntryPoint>>
function foo(): void {
  $foo = readonly new Foo();
  var_dump($foo->getInt());
}
