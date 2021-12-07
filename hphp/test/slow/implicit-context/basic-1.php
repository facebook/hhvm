<?hh

function addFive()[zoned] {
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';

  var_dump(IntContext::start(5, addFive<>));
}
