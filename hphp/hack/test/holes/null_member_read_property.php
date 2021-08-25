<?hh

function null_member_read_property_null(null $xs) : void {
  /* HH_FIXME[4064] */
  $xs->c;
}

function null_member_read_property_nullable(?string $xs) : void {
  /* HH_FIXME[4064] */
  $xs->c;
}

function null_member_read_property_mixed(mixed $xs) : void {
  /* HH_FIXME[4064] */
  $xs->c;
}
