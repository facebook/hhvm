<?hh

function array_append_set(bool $x): void {
  $xs = Set {};
  /* HH_FIXME[4324] */
  $xs[] = $x;
}
