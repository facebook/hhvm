<?hh

function array_get_index_keyset(mixed $idx): void {
  $xs = keyset[];
  /* HH_FIXME[4298] */
  $xs[$idx];
}
