<?hh

class C {
  public function __construct(public ?int $x) {}
}

function g(C $c): int {
  return 1;
}

function h(): int {
  return 1;
}

function test(C $c): void {
  if ($c->x is null) {
  } else {
    $z2 = ($x ==> $c->x + 1)(h());
  }
  if ($c->x is null) {
  } else {
    $z1 = (($x, $y) ==> $c->x + 1)(1, g($c));
  }
}
