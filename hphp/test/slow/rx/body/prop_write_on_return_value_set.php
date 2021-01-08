<?hh

<<__Rx>>
function returns_object() {
  return new stdClass();
}

<<__EntryPoint, __Rx>>
function bad() {
  returns_object()->q = 2;
}
