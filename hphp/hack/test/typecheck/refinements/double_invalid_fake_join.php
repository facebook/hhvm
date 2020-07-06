<?hh // strict

class X {
  public ?int $i;
  public ?string $s;
}

function foo(): void {}

function testit(X $x): int {
  $x->i = 0;
  foo();

  if (1 == 0) {
    $x->i = 42;
  } else {
    $x->s = "hi";
  }

  // Type error should mention $x->i refinement going out of scope.
  return $x->i;
}
