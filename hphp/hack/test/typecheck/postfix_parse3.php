<?hh

class Foo {
  public function __construct(
    public int $a = 1,
  ) {}
}

function f(int $a, int $b, int $c) {
  // This checks that we are parsing it as $a && ($b++ === $c) and not ($a &&
  // $b++) === $c. I.e. the occurrence of a postfix operator should have no
  // impact on the parse of the rest of the expression. If we misparsed this,
  // we should raise a trivial comparison error because it would appear that
  // a boolean value (i.e. $a && b++) was getting compared to an int.
  $a && $b++ === $c;

  // This checks that we are not parsing it as `($a && $a)++`;
  // otherwise it would raise a type error (that a bool is getting incremented)
  if ($a && $a++) {}

  $x = new Foo();

  // should not error
  $x->a++;
}
