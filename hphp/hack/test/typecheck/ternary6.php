<?hh

function takes_string(string $x) {}

function g(): ?string {
  // UNSAFE
}

class Foo {
  public function h(): array<string> {
    // UNSAFE
  }
}

function f($x, Foo $y) {
  if (true) {}
  /* We used to unify both branches of a ternary expression, then falling back
   * to creating a Tunresolved if unification failed; this would hide errors
   * like the one below.
   */
  $a = $x ? array(g()) : $y->h();
  takes_string($a[0]);
}
