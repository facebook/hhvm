<?hh

function printImplicit() {
  var_dump(IntContext::getContext());
  return 0;
}

function addFive() {
  return IntContext::getContext() + IntContext::start(4, printImplicit<>);
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';

  var_dump(IntContext::start(5, addFive<>));
}
