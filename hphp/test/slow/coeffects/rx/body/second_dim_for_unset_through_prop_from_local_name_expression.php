<?hh

<<__EntryPoint>>
function bad()[rx] {
  $p1 = new stdClass();
  $p1->q = darray[4 => true];
  $o = new stdClass();
  $o->p = $p1;

  unset($o->p->{__hhvm_intrinsics\launder_value('q')}[4]);
}
