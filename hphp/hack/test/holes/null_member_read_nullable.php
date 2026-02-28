<?hh

class MyVeryOwnC {
  public function __construct(public int $x) {}
  public function bar(): void {}
}

function null_member_read_nullable(bool $a): void {
  $c = $a ? null : new MyVeryOwnC(0);
  /* HH_FIXME[4064] */
  $c->x;
  /* HH_FIXME[4064] */
  $c->bar();
}
