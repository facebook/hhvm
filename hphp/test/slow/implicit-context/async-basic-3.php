<?hh

async function printImplicit() {
  echo "Implicit: " . (string) IntContext::getContext() . "\n";
}

async function aux() {
  $x = IntContext::getContext();
  var_dump($x);
  await IntContext::genStart($x+1, printImplicit<>);
  var_dump(IntContext::getContext());
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  await IntContext::genStart(0, aux<>);
}
