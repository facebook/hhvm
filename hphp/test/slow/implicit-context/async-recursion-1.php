<?hh

async function aux() {
  $x = IntContext::getContext();
  if ($x > 10) return;
  var_dump($x);
  await IntContext::genStart($x+1, aux<>);
  var_dump(IntContext::getContext());
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  await IntContext::genStart(0, aux<>);
}
