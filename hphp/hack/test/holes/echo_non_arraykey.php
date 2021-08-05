<?hh

function echo_non_arraykey(mixed $x, bool $y) : void {
  /* HH_FIXME[4438] */
  echo($x);
  /* HH_FIXME[4438] */
  echo($y);
}
