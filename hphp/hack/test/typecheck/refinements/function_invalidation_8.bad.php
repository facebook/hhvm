<?hh

class C {
  public ?int $i = null;

  public function harmful()[write_props]: void {}
}

function takes_int(int $i)[]: void {}

function refine_ko(C $c): void {
  if ($c->i is nonnull) {
    $c->harmful();
    takes_int($c->i);
  }
}
