<?hh

function array_get_index_map_nothing(mixed $idx): void {
  $xs = Map {};
  /* HH_FIXME[4298] */
  /* HH_FIXME[4324] */
  $xs[$idx];
}

function array_get_index_map_string(Map<string,int> $xs, mixed $idx): void {
  /* HH_FIXME[4298] */
  /* HH_FIXME[4324] */
  $xs[$idx];
}
