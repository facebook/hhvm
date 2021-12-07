<?hh

async function f()[zoned] {
  echo ClassContext::getContext()->name() . "\n";
  echo ClassContext::getContext()->name() . "\n";
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  await ClassContext::genStart(new C(), f<>);
}
