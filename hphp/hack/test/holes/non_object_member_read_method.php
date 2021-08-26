<?hh

function non_object_member_read_method(nonnull $xs) : void {
  /* HH_FIXME[4062] */
  $xs->c();
}
