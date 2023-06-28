<?hh

async function f()[zoned] :Awaitable<mixed>{
  echo 'Expecting C got ' . ClassContext::getContext()->name() . "\n";
  HH\Coeffects\backdoor(()[defaults] ==> set_time_limit(1));
  // throw C++ exception so that the context does not get unset
  while(1) {}
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  include 'async-implicit.inc';

  register_postsend_function(() ==> {
    try {
      $name = ClassContext::getContext()->name();
      echo 'Fail: got context ' . $name . "\n";
    } catch (TypeAssertionException $_) {
      echo "Correct: no context!\n";
    }
  });
  await ClassContext::genStart(new C, f<>);
}
