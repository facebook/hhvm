<?hh // partial

function takes_string(string $x) {}

/* HH_FIXME[4110] */
function g(): ?string {

}

class Foo {
  /* HH_FIXME[4110] */
  public function h(): array<string> {

  }
}

function f($x, Foo $y) {
  if (true) {}
  /* We used to unify both branches of a ternary expression, then falling back
   * to creating a Tunion if unification failed; this would hide errors
   * like the one below.
   */
  $a = $x ? varray[g()] : $y->h();
  takes_string($a[0]);
}
