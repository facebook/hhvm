<?hh

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  var_dump(IntContext::getContext());
}
