<?hh

function array_append_keyset(bool $x): void {
  $xs = keyset[];
  /* HH_FIXME[4324] */
  $xs[] = $x;
}

function array_append_keyset_string_opt(?string $x): void {
  $xs = keyset[];
  /* HH_FIXME[4324] */
  $xs[] = $x;
}
