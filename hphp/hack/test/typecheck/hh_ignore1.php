<?hh

function f9(?int $x): void {
  /* HH_IGNORE[12003] ignore on warning -> ok */
  $x ? $x : 0;
  /* HH_FIXME[12003] fixme on warning -> nok */
  $x ? $x : 0;
  /* HH_IGNORE[2050] ignore on error -> nok */
  $y;
}
