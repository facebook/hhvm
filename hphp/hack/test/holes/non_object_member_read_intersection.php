<?hh

function non_object_member_read_method_intersection((int & arraykey) $xs) : void {
  /* HH_FIXME[4062] */
  $xs->c();
}
