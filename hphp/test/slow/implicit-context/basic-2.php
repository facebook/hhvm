<?hh

function printImplicit() :mixed{
  var_dump(IntContext::getContext());
  return 0;
}

function addFive() :mixed{
  return IntContext::getContext() + IntContext::start(4, printImplicit<>);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  var_dump(IntContext::start(5, addFive<>));
}
