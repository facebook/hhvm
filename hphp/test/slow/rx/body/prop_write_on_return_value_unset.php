<?hh

<<__Rx>>
function returns_object() {
  return new stdClass();
}

<<__EntryPoint, __Rx>>
function bad() {
  unset(returns_object()->q);
}
