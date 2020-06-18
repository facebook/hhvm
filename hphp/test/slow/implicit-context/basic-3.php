<?hh

include 'implicit.inc';

function printImplicit() {
  echo "Implicit: " . (string) IntContext::getContext() . "\n";
}

function aux() {
  $x = IntContext::getContext();
  var_dump($x);
  IntContext::start($x+1, fun('printImplicit'));
  var_dump(IntContext::getContext());
}

<<__EntryPoint>>
function main() {
  IntContext::start(0, fun('aux'));
}
