<?hh
class Foo {
  public ?Foo $x = null;
  public readonly function getFoo(?Foo $x): ?Foo {
    return  $x ?? $this->x; // error, could be readonly
  }
}
