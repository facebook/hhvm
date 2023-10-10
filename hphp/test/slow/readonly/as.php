<?hh

class Bar {}

class Foo extends Bar {
  public function __construct(public int $prop = 3)[] {}
}

function main(readonly Foo $ro, Foo $mut, bool $b): Foo {
  $x = $ro as Bar;
  return $x; // error, $x is readonly
}
