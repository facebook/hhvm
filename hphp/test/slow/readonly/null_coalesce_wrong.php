<?hh // strict
class Foo {
  public ?Foo $x = null;
  public readonly function getFoo(?Foo $x): ?Foo {
    return  $x ?? $this->x; // error, could be readonly (should not error yet as of D32573602)
  }
}
<<__EntryPoint>>
function foo(): void {
  echo "Ok!\n";
}
