<?hh

function variadic(int $x, float ...$xs): void {}

function call_variadic(int $x, float $y): void {
  /* HH_FIXME[4110] */
  variadic($x, $x);

  /* HH_FIXME[4110] */
  variadic($x, $y, $x);

  /* HH_FIXME[4110] */
  variadic($x, $x, $y);

  /* HH_FIXME[4110] */
  variadic($x, $x, $x);
}
