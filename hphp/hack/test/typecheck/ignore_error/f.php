<?hh

function foo(mixed $m): void {
  /* HH_IGNORE_ERROR[4063] */
  $m[3];

  /* HH_FIXME[4063] */
  $m[3];
}
