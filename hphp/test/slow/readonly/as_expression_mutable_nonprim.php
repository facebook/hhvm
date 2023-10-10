<?hh
class Foo {
  public int $x = 4;
  public function getFoo(readonly Foo $x): Foo {
    return $x as Foo; // error
  }
}
