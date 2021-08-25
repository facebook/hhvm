<?hh

function array_access_write_union_vector((bool | Vector<string>) $xs): void {
  /* HH_FIXME[4370] */
  /* HH_FIXME[4110] */
  ($xs)[0] = 1;
}

function array_access_write_union_vec((bool | vec<string>) $xs): void {
  /* HH_FIXME[4370] */
  /* HH_FIXME[4110] */
  $xs[0] = 1;
}
