<?hh

function printImplicit() :mixed{
  var_dump(ClassContext::getContext()->getPayload());
  return 0;
}

function addFive() :mixed{
  return ClassContext::getContext()->getPayload() + ClassContext::start(new Base(4), printImplicit<>);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  var_dump(ClassContext::start(new Base(5), addFive<>));
}
