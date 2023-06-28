<?hh

async function f()[zoned] :Awaitable<mixed>{
  echo ClassContext::getContext()->name() . "\n";
  echo ClassContext::getContext()->name() . "\n";
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  await ClassContext::genStart(new C(), f<>);
}
