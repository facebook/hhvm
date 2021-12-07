<?hh

async function printImplicit()[zoned] {
  var_dump(IntContext::getContext());
  return 0;
}

async function addFive()[zoned] {
  return IntContext::getContext() + await IntContext::genStart(4, printImplicit<>);
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  var_dump(await IntContext::genStart(5, addFive<>));
}
