<?hh // partial

function takes_int(int $x) { }

function f() {
  $a = 1;
  if (true) {} // this makes $a into a Tvar mapped to a Tunion[int]
  /* We used to unify both branches of a ternary expression, then falling back
   * to creating a Tunion if unification failed; this would create a false
   * positive error for the code below.
   */
  true ? null : $a;
  takes_int($a);
}
