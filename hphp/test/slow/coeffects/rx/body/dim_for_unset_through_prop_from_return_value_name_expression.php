<?hh

function returns_object()[rx] {
  $o = new stdClass();
  $o->p = darray[5 => true];
  return $o;
}

<<__EntryPoint>>
function bad()[rx] {
  unset(returns_object()->{__hhvm_intrinsics\launder_value('p')}[5]);
}
