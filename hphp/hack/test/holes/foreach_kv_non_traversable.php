<?hh

function foreach_kv_non_traversable(int $xs): void {
  /* HH_FIXME[4110] */
  foreach ($xs as $k => $v) {
  }
}
