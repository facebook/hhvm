<?hh

class C {
  public function __construct(public ?int $x) {}
}

function g(C $c): int {
  return 1;
}

function f(C $c): void {
  if ($c->x is null) {
  } else {
    (() ==> $c->x + 1) ();
  }
  if ($c->x is null) {
  } else {
    (($x) ==> $c->x + 1) (1);
  }
  if ($c->x is null) {
  } else {
    1 |> (($x, $y, $z) ==> $c->x + 1) (vec[1], $$, $c);
  }
  if ($c->x is null) {
  } else {
    $z1 = ($x ==> g($c))($c->x + 1);
  }
}
