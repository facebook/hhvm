<?hh

async function printImplicit()[zoned] :Awaitable<mixed>{
  var_dump(IntContext::getContext()->getPayload());
  return 0;
}

async function addFive()[zoned] :Awaitable<mixed>{
  return IntContext::getContext()->getPayload() + await IntContext::genStart(new Base(4), printImplicit<>);
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  var_dump(await IntContext::genStart(new Base(5), addFive<>));
}
