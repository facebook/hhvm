<?hh

function append_string_datetime(string $x, DateTime $y): string {
  /* HH_FIXME[4067] */
  return $x.$y;
}
