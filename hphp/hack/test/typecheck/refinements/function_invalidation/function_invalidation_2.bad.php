<?hh

class C {
  public ?int $i = null;
}

function harmful2(C $c)[write_props]: void {}

function takes_int(int $i)[]: void {}

function refine_ko(C $c): void {
  if ($c->i is nonnull) {
    harmful2($c);
    takes_int($c->i);
  }
}
