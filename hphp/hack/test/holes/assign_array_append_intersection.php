<?hh

function assign_array_append_intersection((int & string) $x, Vector<(int & float)> $xs) : void {
  /* HH_FIXME[4110] */
  $xs[] = $x;
}
