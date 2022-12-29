<?hh

function f(string $a): string {
  $b = /* HH_FIXME[4110] */ $a + 1;
  /* HH_FIXME[4110] */
  return $b;
}

function g(string $a): string {
  /* HH_FIXME[4110] */
  $b = $a + 1;
  /* HH_FIXME[4110] */
  return $b;
}
