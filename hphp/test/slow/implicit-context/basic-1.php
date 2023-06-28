<?hh

function addFive()[zoned] :mixed{
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  var_dump(IntContext::start(5, addFive<>));
}
