<?hh

async function printImplicit() :Awaitable<mixed>{
  echo "Implicit: " . (string) IntContext::getContext() . "\n";
}

async function aux() :Awaitable<mixed>{
  $x = IntContext::getContext();
  var_dump($x);
  await IntContext::genStart($x+1, printImplicit<>);
  var_dump(IntContext::getContext());
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  await IntContext::genStart(0, aux<>);
}
