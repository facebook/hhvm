<?hh

function array_get_index_dict(mixed $idx): void {
  $xs = dict[];
  /* HH_FIXME[4298] */
  $xs[$idx];
}
