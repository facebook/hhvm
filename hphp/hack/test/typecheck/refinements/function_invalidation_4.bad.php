<?hh

class C {
  public ?int $i = null;
}

function takes_int(int $i)[]: void {}

function refine_ko(C $c, dynamic $harmful): void {
  if ($c->i is nonnull) {
    $harmful();
    takes_int($c->i);
  }
}
