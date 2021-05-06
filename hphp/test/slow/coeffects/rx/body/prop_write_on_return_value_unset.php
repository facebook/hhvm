<?hh

function returns_object()[rx] {
  return new stdClass();
}

<<__EntryPoint>>
function bad()[rx] {
  unset(returns_object()->q);
}
