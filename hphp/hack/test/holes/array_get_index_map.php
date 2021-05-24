<?hh

function array_get_index_map(mixed $idx): void {
  $xs = Map {};
  /* HH_FIXME[4298] */
  $xs[$idx];
}
