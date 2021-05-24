<?hh

function assign_array_get_index_string(string $xs, mixed $idx): void {
  /* HH_FIXME[4324] */
  $xs[$idx] = "a";
}
