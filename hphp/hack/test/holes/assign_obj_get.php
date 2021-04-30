<?hh

class C {
  public arraykey $prop = 0;
}

function assign_object_get(C $x) : void {
  /* HH_FIXME[4110] */
  $x->prop = false;
}
