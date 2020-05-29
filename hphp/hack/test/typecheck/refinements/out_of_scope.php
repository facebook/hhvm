<?hh // strict

class C {
  public ?int $i;
  public ?int $j;
}

function foo(): void {}

function testit(C $c): int {
  if (1 == 2) {
    assert($c->j is nonnull);
    foo();
    assert($c->i is nonnull);
  }

  return $c->i;
}
