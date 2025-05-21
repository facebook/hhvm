<?hh

async function aux()[zoned] :Awaitable<mixed>{
  $x = IntContext::getContext()->getPayload();
  if ($x > 10) return;
  var_dump($x);
  await IntContext::genStart(new Base($x+1), aux<>);
  var_dump(IntContext::getContext()->getPayload());
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  await IntContext::genStart(new Base(0), aux<>);
}
