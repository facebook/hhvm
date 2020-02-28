<?hh

<<__Rx, __MutableReturn>>
function returns_object() {
  $o = new stdClass();
  $o->p = darray[5 => true];
  return $o;
}

<<__EntryPoint, __Rx>>
function bad() {
  $lp = 'p';
  unset(returns_object()->{$lp}[5]);
}
