<?hh // strict

class C {
  public ?int $i;
  public ?int $j;
}

function foo(): void {}

function testit(C $c): int {
  if (1 == 2) {
    invariant($c->j is nonnull, "");
    foo();
    invariant($c->i is nonnull, "");
  }

  return $c->i;
}
