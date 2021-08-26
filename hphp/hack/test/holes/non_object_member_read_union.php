<?hh

function non_object_member_read_method_union((int | bool) $xs) : void {
  /* HH_FIXME[4062] */
  $xs->c();
}
