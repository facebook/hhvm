<?hh

function array_access_write_intersection((arraykey & string) $xs): void {
  /* HH_FIXME[4110] */
  $xs[0] = 1;
}

function array_access_write_intersection_empty((arraykey & int) $xs): void {
  /* HH_FIXME[4110] */
  /* HH_FIXME[4370] */
  $xs[0] = 1;
}
