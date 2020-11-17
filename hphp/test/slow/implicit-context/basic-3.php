<?hh

function printImplicit() {
  echo "Implicit: " . (string) IntContext::getContext() . "\n";
}

function aux() {
  $x = IntContext::getContext();
  var_dump($x);
  IntContext::start($x+1, printImplicit<>);
  var_dump(IntContext::getContext());
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';

  IntContext::start(0, aux<>);
}
