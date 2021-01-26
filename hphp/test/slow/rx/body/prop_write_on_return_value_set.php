<?hh

function returns_object()[rx] {
  return new stdClass();
}

<<__EntryPoint>>
function bad()[rx] {
  returns_object()->q = 2;
}
