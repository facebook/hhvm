<?hh

class Foo {
  public function __construct(public int $prop = 3)[] {}
}

function main(readonly Foo $ro, Foo $mut, bool $b): (Foo, Foo) {
  $x = tuple($ro, $mut);
  return $x; // error, $x is readonly
}
