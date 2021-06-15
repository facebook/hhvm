<?hh

class Harmful {
  public function __construct()[write_props]: void {}
}

class C {
  public ?int $i = null;
}

function takes_int(int $i)[]: void {}

function refine_ko(C $c): void {
  if ($c->i is nonnull) {
    new Harmful();
    takes_int($c->i);
  }
}
