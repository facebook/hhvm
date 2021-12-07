<?hh // strict
class Foo {
  public ?Foo $x = null;
  public readonly function getFoo(): Foo {
    return $this->x ?? new Foo(); // error, could ber readonly
  }
}
