<?hh

include 'async-implicit.inc';

<<__EntryPoint>>
async function main() {
  var_dump(IntContext::getContext());
}
