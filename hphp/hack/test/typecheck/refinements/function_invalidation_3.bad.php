<?hh

class C {
  public ?int $i = null;
}

function harmful3(C $c)[policied]: void {}

function takes_int(int $i)[]: void {}

function refine_ko(C $c): void {
  if ($c->i is nonnull) {
    harmful3($c);
    takes_int($c->i);
  }
}
