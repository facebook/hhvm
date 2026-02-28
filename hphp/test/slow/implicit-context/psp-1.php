<?hh

async function f()[zoned] :Awaitable<mixed>{
  try {
    $ctx = ClassContext::getContext()->name();
  } catch (Exception $e) {
    $ctx = $e->getMessage();
  }
  echo 'Expecting C got ' . $ctx . "\n";
  HH\Coeffects\backdoor(()[defaults] ==> set_time_limit(1));
  // throw C++ exception so that the context does not get unset
  while(1) {}
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  register_postsend_function(() ==> {
    $name = ClassContext::getContext()?->name() ?? 'null';
    echo 'Got context ' . $name . "\n";
  });
  await ClassContext::genStart(new C, f<>);
}
