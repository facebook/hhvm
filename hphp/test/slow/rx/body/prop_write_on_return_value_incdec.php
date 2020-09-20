<?hh

<<__Rx, __MutableReturn>>
function returns_object() {
  return new stdClass();
}

<<__EntryPoint, __Rx>>
function bad() {
  returns_object()->q++;
}
