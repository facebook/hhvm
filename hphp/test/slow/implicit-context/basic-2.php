<?hh

include 'implicit.inc';

function printImplicit() {
  var_dump(IntContext::getContext());
}

function addFive() {
  return IntContext::getContext()
    + IntContext::start(4, fun('printImplicit'));
}

<<__EntryPoint>>
function main() {
  var_dump(IntContext::start(5, fun('addFive')));
}
