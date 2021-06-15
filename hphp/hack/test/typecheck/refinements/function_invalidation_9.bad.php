<?hh

class C {
  public ?int $i = null;

  public static function staticHarmful()[write_props]: void {}
}

function takes_int(int $i)[]: void {}

function refine_ko(C $c): void {
  if ($c->i is nonnull) {
    C::staticHarmful();
    takes_int($c->i);
  }
}
