<?hh

async function printImplicit() :Awaitable<mixed>{
  echo "Implicit: " . (string) IntContext::getContext()->getPayload() . "\n";
}

async function aux() :Awaitable<mixed>{
  $x = IntContext::getContext()->getPayload();
  var_dump($x);
  await IntContext::genStart(new Base($x+1), printImplicit<>);
  var_dump(IntContext::getContext()->getPayload());
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  await IntContext::genStart(new Base(0), aux<>);
}
