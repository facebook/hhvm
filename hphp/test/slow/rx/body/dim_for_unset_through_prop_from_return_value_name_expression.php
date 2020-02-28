<?hh

<<__Rx, __MutableReturn>>
function returns_object() {
  $o = new stdClass();
  $o->p = darray[5 => true];
  return $o;
}

<<__EntryPoint, __Rx>>
function bad() {
  unset(returns_object()->{__hhvm_intrinsics\launder_value('p')}[5]);
}
