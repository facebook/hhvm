<?hh

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';
  try {
    var_dump(IntContext::getContext());
  } catch (InvalidOperationException $e) {
    echo $e->getMessage();
  }
}
