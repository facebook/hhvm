<?hh

<<__EntryPoint, __Rx>>
function bad() {
  $p1 = new stdClass();
  $p1->q = darray[4 => true];
  $o = new stdClass();
  $o->p = $p1;

  $o->p->q[4] = false;
}
