<?hh

include 'implicit.inc';

function addFive() {
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
function main() {
  var_dump(IntContext::start(5, fun('addFive')));
}
