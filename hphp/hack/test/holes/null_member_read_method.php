<?hh

function null_member_read_method_null(null $xs) : void {
  /* HH_FIXME[4064] */
  $xs->c();
}

function null_member_read_method_nullable(?string $xs) : void {
  /* HH_FIXME[4064] */
  $xs->c();
}

function null_member_read_method_mixed(mixed $xs) : void {
  /* HH_FIXME[4064] */
  $xs->c();
}
