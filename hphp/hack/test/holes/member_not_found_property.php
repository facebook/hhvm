<?hh

class C {}

function  member_not_found_property_fixme(C $c) : void {
  /* HH_FIXME[4053] */
  $c->does_not_exist;
}


function  member_not_found_property_no_fixme(C $c) : void {
  $c->does_not_exist;
}
