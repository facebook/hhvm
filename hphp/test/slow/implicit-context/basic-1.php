<?hh

function addFive()[zoned] :mixed{
  return ClassContext::getContext()->getPayload() + 5;
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  var_dump(ClassContext::start(new Base(5), addFive<>));
}
