<?hh

function printImplicit() :mixed{
  echo "Implicit: " . (string) ClassContext::getContext()->getPayload() . "\n";
}

function aux() :mixed{
  $x = ClassContext::getContext()->getPayload();
  var_dump($x);
  ClassContext::start(new Base($x+1), printImplicit<>);
  var_dump(ClassContext::getContext()->getPayload());
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';

  ClassContext::start(new Base(0), aux<>);
}
