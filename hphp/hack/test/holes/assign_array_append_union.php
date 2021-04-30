<?hh

function assign_array_append_union((int|string) $x, Vector<(int|bool)> $xs) : void {
  /* HH_FIXME[4110] */
  $xs[] = $x;
}
