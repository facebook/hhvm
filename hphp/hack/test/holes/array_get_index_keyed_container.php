<?hh

function array_get_index_keyed_container(
  KeyedContainer<string, int> $xs,
  mixed $idx,
): void {
  /* HH_FIXME[4298] */
  $xs[$idx];
}
