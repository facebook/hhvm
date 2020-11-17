<?hh

async function f() {
  try {
    ClassContext::getContext();
  } catch (TypeAssertionException $_) {
    echo "Success: f no context!\n";
  }
}

<<__NoContext>>
async function g() {
  try {
    ClassContext::getContext();
  } catch (TypeAssertionException $_) {
    echo "Success: g no context!\n";
  }
}

<<__EntryPoint>>
async function main() {
  include 'async-implicit.inc';

  HH\gen_without_implicit_context(f<>);
  await ClassContext::genStart(new C, async () ==> {
    await HH\gen_without_implicit_context(f<>);
  });
  await HH\gen_without_implicit_context(g<>);
  try {
    await ClassContext::genStart(new C, g<>);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  await ClassContext::genStart(new C, async () ==> {
    await HH\gen_without_implicit_context(g<>);
  });
}
