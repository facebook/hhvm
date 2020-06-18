<?hh

include 'async-implicit.inc';

async function printImplicit() {
  var_dump(IntContext::getContext());
}

async function addFive() {
  return IntContext::getContext()
    + await IntContext::genStart(4, fun('printImplicit'));
}

<<__EntryPoint>>
async function main() {
  var_dump(await IntContext::genStart(5, fun('addFive')));
}
