<?hh

function array_append_keyset(bool $x): void {
  $xs = keyset[];
  /* HH_FIXME[4435] */
  $xs[] = $x;
}
