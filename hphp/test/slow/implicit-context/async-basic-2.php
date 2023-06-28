<?hh

async function printImplicit()[zoned] :Awaitable<mixed>{
  var_dump(IntContext::getContext());
  return 0;
}

async function addFive()[zoned] :Awaitable<mixed>{
  return IntContext::getContext() + await IntContext::genStart(4, printImplicit<>);
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  var_dump(await IntContext::genStart(5, addFive<>));
}
