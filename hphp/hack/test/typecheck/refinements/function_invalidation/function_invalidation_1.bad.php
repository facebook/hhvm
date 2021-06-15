<?hh

class C {
  public ?int $i = null;
}

function harmful1(C $c): void {}

function takes_int(int $i)[]: void {}

function refine_ko(C $c): void {
  if ($c->i is nonnull) {
    harmful1($c);
    takes_int($c->i);
  }
}
