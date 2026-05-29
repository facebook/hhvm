<?hh

function f9(int $x): void {
  /* HH_IGNORE[12009] ignore on warning -> ok */
  $x is int;
  /* HH_FIXME[12009] fixme on warning -> nok */
  $x is int;
  /* HH_IGNORE[2050] ignore on error -> nok */
  $y;
}
