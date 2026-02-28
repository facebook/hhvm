<?hh

class Foo {
  public function __construct(public int $prop = 3)[] {}
}

function main(readonly Foo $ro, Foo $mut, bool $b): Foo {
  $x = $b ? $mut : $ro;
  return $x; // error, $x is readonly
}
