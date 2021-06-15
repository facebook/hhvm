<?hh

class C {
  public ?int $i = null;
}

function takes_int(int $i)[]: void {}

function refine_ko(C $c): void {
  if ($c->i is nonnull) {
    $harmful5 = (1 === 2) ? ()[] ==> {} : ()[write_props] ==> {};
    $harmful5();
    takes_int($c->i);
  }
}
