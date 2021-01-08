<?hh

<<__Rx>>
function returns_object() {
  $o = new stdClass();
  $o->p = darray[5 => true];
  return $o;
}

<<__EntryPoint, __Rx>>
function bad() {
  returns_object()->{__hhvm_intrinsics\launder_value('p')}[5] = false;
}
