<?hh

function assign_array_get_index_dict(mixed $idx): void {
  $xs = dict[];
  /* HH_FIXME[4371] */
  $xs[$idx] = 1;
}
