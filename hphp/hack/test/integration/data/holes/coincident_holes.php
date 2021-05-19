<?hh

function foo(bool $x): string {
  /* HH_FIXME[4110] */
  return $x + 1 ? '1' : 1;
}
