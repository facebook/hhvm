<?hh

include 'async-implicit.inc';

async function f() {
  echo 'Expecting C got ' . ClassContext::getContext()->name() . "\n";
  set_time_limit(1);
  // throw C++ exception so that the context does not get unset
  while(1) {}
}

<<__EntryPoint>>
async function main() {
  register_postsend_function(() ==> {
    try {
      $name = ClassContext::getContext()->name();
      echo 'Fail: got context ' . $name . "\n";
    } catch (TypeAssertionException $_) {
      echo "Correct: no context!\n";
    }
  });
  await ClassContext::genStart(new C, fun('f'));
}
