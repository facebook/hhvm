<?hh

class Foo {
  public function __construct(public int $prop = 3)[] {}
}

function main(readonly Foo $ro, Foo $mut, bool $b): Pair<Foo, Foo> {
  $x = Pair {$ro, $mut};
  return $x; // error, $x is readonly
}
