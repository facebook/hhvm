<?hh

include 'async-implicit.inc';

async function f() {
  echo ClassContext::getContext()->name() . "\n";
  echo ClassContext::getContext()->name() . "\n";
}

<<__EntryPoint>>
async function main() {
  await ClassContext::genStart(new C(), fun('f'));
}
