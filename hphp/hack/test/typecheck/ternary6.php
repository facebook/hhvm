<?hh

function takes_string(string $x): void {}

/* HH_FIXME[4110] */
function g(): ?string {

}

class Foo {
  /* HH_FIXME[4110] */
  public function h(): varray<string> {

  }
}

function f(bool $x, Foo $y): void {
  if (true) {}
  /* We used to unify both branches of a ternary expression, then falling back
   * to creating a Tunion if unification failed; this would hide errors
   * like the one below.
   */
  $a = $x ? vec[g()] : $y->h();
  takes_string($a[0]);
}
