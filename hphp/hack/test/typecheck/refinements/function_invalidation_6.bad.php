<?hh

class C {
  public ?int $i = null;
}
function takes_int(int $i)[]: void {}

function refine_ko(C $c): void {
  if ($c->i is nonnull) {
    $harmful = (1 === 2) ? () ==> {} : ()[] ==> {};
    $harmful();
    takes_int($c->i);
  }
}
