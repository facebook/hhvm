<?hh

class C {}

function member_not_found_method(C $c): void {
  /* HH_FIXME[4053] */
  $c->does_not_exist();
}
