<?hh

async function printImplicit() {
  var_dump(IntContext::getContext());
}

async function addFive() {
  return IntContext::getContext()
    + await IntContext::genStart(4, printImplicit<>);
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  var_dump(await IntContext::genStart(5, addFive<>));
}
