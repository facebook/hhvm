<?hh

function assign_array_get_index_map(mixed $idx): void {
  $xs = Map {};
  /* HH_FIXME[4371] */
  $xs[$idx] = 1;
}
