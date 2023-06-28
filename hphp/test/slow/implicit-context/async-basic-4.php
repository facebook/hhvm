<?hh

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  var_dump(IntContext::getContext());
}
