<?hh

function non_object_member_read_property(nonnull $xs) : void {
  /* HH_FIXME[4062] */
  $xs->c;
}
