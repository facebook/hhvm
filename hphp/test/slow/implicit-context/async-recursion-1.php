<?hh

include 'async-implicit.inc';

async function aux() {
  $x = IntContext::getContext();
  if ($x > 10) return;
  var_dump($x);
  await IntContext::genStart($x+1, fun('aux'));
  var_dump(IntContext::getContext());
}

<<__EntryPoint>>
async function main() {
  await IntContext::genStart(0, fun('aux'));
}
