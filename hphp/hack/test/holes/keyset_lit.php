<?hh

function keyset_lit(): void {
  /* HH_FIXME[4110] */
  $x = keyset[1, true];
}

function keyset_lit_string_opt(?string $x) : void {
  /* HH_FIXME[4110] */
  $y = keyset[$x];
}
